#include <barelib.h>
#include <malloc.h>
#include <thread.h>

#include <bareio.h>

extern uint32* mem_start;
extern uint32* mem_end;
static alloc_t* freelist;

/*  Sets the 'freelist' to 'mem_start' and creates  *
 *  a free allocation at that location for the      *
 *  entire heap.                                    */
//--------- This function is complete --------------//
void heap_init(void) {
  freelist = (alloc_t*)mem_start;
  freelist->size = get_stack(NTHREADS) - mem_start - sizeof(alloc_t);
  freelist->state = M_FREE;
  freelist->next = NULL;
}


// Merge current free with its next free
// assumes that free and free->next are both free
void _merge_alloc(alloc_t *free){
  if(free->size != 0)
    debug_printf(DEBUG_MALLOC, "merge alloc\n");
  free->size += free->next->size;
  free->next = free->next->next; 
}

void _print_alloc_list(void){
  alloc_t *cur = freelist;
  while(cur != NULL){
    debug_printf(DEBUG_MALLOC, "\t\t(%x, %d, %s)\n", cur, cur->size, (cur->state == M_FREE) ? "free" : "used");
    cur = cur->next;
  }
}

void public_print_alloc_list([[maybe_unused]] char *args){
  alloc_t *cur = freelist;
  while(cur != NULL){
    printf("\033[93m");
    printf("\t\t(%x, %d, %s)\n", cur, cur->size, (cur->state == M_FREE) ? "free" : "used");
    cur = cur->next;
  }
  printf("\033[39m");
}


/*  Locates a free block large enough to contain a new    *
 *  allocation of size 'size'.  Once located, remove the  *
 *  block from the freelist and ensure the freelist       *
 *  contains the remaining free space on the heap.        *
 *  Returns a pointer to the newly created allocation     */
void* malloc(uint64 size) {
  uint64 space = size + sizeof(alloc_t);
  alloc_t *cur = freelist;

  while(!(cur->state == M_FREE && cur->size >= size)){ // while I can't find an unused alloc with enough room for me and my phat ass
    if(cur->next == NULL){ // no memory :(
      debug_printf(DEBUG_MALLOC, "MALLOC FAILURE!!! MALLOC FAILURE!!! MALLOC FAILURE!!!\n");
      return NULL; 
    }
    // else if(cur->state == M_FREE && cur->next->state == M_FREE) _merge_alloc(cur);
    else cur = cur->next;
  }

  void *ret = (void*)((uint64)cur + (uint64)sizeof(alloc_t));
  if(cur->size == size){ // my malloc fits perfectly into that free spot
    cur->state = M_USED;
    return ret;
  }

  alloc_t *next = cur->next;

  cur->next = (alloc_t*)((uint64)cur+space);       // update 
  cur->next->next = next;                  // pointers

  cur->next->size = cur->size - space;     // update
  cur->size = size;                        // sizes

  cur->state = M_USED;                     // update
  cur->next->state = M_FREE;               // states

  if(cur == freelist) freelist = cur -> next;

  debug_printf(DEBUG_MALLOC, "malloc(%d)\n", size);
  _print_alloc_list();

  return ret;
}

/*  Free the allocation at location 'addr'.  If the newly *
 *  freed allocation is adjacent to another free          *
 *  allocation, coallesce the adjacent free blocks into   *
 *  one larger free block.                                */
void free(void* addr) {
  if((void*)addr > (void*)mem_end) return;
  alloc_t *free_entry = (alloc_t*)((uint64)addr - (uint64)sizeof(alloc_t));
  free_entry->state = M_FREE;

  debug_printf(DEBUG_MALLOC, "free(%x)\n", free_entry);
  _print_alloc_list();

  // if(free_entry->next != NULL && free_entry->next->state == M_FREE) _merge_alloc(free_entry);

  debug_printf(DEBUG_MALLOC, "after merge:\n", free_entry);
  _print_alloc_list();

  alloc_t *prev = freelist;
  alloc_t *cur = freelist->next;
  while(cur != NULL && (uint64)(cur) < (uint64)(free_entry)){
    prev = cur;
    cur = cur->next;
  }

  free_entry->next = prev->next;
  prev->next = free_entry;

  uint64 prev_addr = (uint64)prev + prev->size + sizeof(alloc_t);
  if(prev_addr == (uint64)free_entry){ // coalesce down
    prev->size += sizeof(alloc_t) + free_entry->size;
    prev->next = free_entry->next;
    free_entry = prev;
  }

  uint64 next_addr = (uint64)free_entry + free_entry->size + sizeof(alloc_t);
  if(next_addr == (uint64)(free_entry->next)){ // coalesce up
    free_entry->size += sizeof(alloc_t) + free_entry->next->size;
    free_entry->next = free_entry->next->next;
  }


  return;
}


void *realloc(void *ptr, uint64 size){
  // According to man
  debug_printf(DEBUG_MALLOC, "realloc(%x, %d)\n", ptr, size);

  if(ptr == NULL) return malloc(size);
  if(ptr != NULL && size == 0){
    free(ptr);
    return NULL;
  } 

  alloc_t *entry = (alloc_t*)((uint64)ptr - (uint64)sizeof(alloc_t));

  if(entry->state == M_FREE){ // reallocing a free ptr
    debug_printf(DEBUG_MALLOC, "\tRealloc free ptr\n");
    return NULL;
  }

  // start merging the entries in front of me
  alloc_t *cur = entry->next;
  if(cur == NULL) goto malloc_and_copy; // end of list so there is nothing ahead of me

  // look ahead to see if I can just merge free spots ahead of me
  while(cur->state == M_FREE && cur->next != NULL && cur->next->state == M_FREE) _merge_alloc(cur);
  
  if(cur->state != M_FREE) goto malloc_and_copy; // can't merge ahead so don't even bother

  uint64 space_needed = size + (uint64)sizeof(alloc_t);
  uint64 space_available = entry->size + cur->size - (uint64)sizeof(alloc_t);
  if(space_needed > space_available) goto malloc_and_copy; // even after merge, I couldn't fit :(

  if(space_needed == space_available){ // no need for intermediary new_next pointer
    entry->size = space_needed;
    entry->next = cur->next;
    _print_alloc_list();
    return ptr;
  }

  // I HAVE ROOM!!!!!!!!!!!!!!
  alloc_t *new_next = entry+space_needed; // create intermediary new_next

  new_next->size = space_available - space_needed + (uint64)sizeof(alloc_t); // update
  entry->size = space_needed;                                                // sizes

  new_next->next = cur->next;                                                // update
  entry->next = new_next;                                                    // pointers

  _print_alloc_list();                                                       // debugging

  return ptr;

  /*  malloc a new pointer that is big enough   *
   *  copy data from ptr to new_ptr             *
   *  free ptr                                  */
  malloc_and_copy:

  void *new_ptr;
  if((new_ptr = malloc(size)) == NULL) return NULL;             // no memory available
  memcpy(new_ptr, ptr, entry->size - (uint64)sizeof(alloc_t));  // copy old contents
  free(ptr);                                                    // free old pointer
  _print_alloc_list();                                          // debugging
  return new_ptr;                                               // return new pointer
}