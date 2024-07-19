#include <barelib.h>
#include <fs.h>
#include <string.h>
#include <malloc.h>
#include <bareio.h>

extern fsystem_t* fsd;
extern filetable_t oft[NUM_FD];

/*  Search for a filename  in the directory, if the file doesn't exist  *
 *  or it is  already  open, return  an error.   Otherwise find a free  *
 *  slot in the open file table and initialize it to the corresponding  *
 *  inode in the root directory.                                        *
 *  'head' is initialized to the start of the file.                     */
int32 fs_open(char* filename) {
  directory_t *root = &(fsd->root_dir);

  debug_printf(DEBUG_FILE, "\n");
  debug_printf(DEBUG_FILE, "Opening file: %s\n", filename);

  // find file
  int dirent_idx = -1;
  for(int i=0; i<DIR_SIZE; ++i){
    dirent_t dirent = root->entry[i];
    if(strcmp(dirent.name, filename) == 0){
      dirent_idx = i;
      debug_printf(DEBUG_FILE, "\t\tFound file in directory at index %d\n", i);
      break; 
    }
  }
  // no such file in directory
  if(dirent_idx == -1){
    debug_printf(DEBUG_FILE, "No such file in directory: %s\n", filename);
    return -1; 
  }
  // check if file is already open
  for(int i=0; i<NUM_FD; ++i){
    if(oft[i].direntry == dirent_idx && oft[i].state == FSTATE_OPEN){
      debug_printf(DEBUG_FILE, "File is already open: %s at index %d\n", filename, i);
      return -1; 
    }
  }
  // read the inode for the file from the ramdisk
  inode_t *inode = (inode_t*)malloc(sizeof(inode_t));
  bs_read(root->entry[dirent_idx].inode_block, 0, (void*)inode, sizeof(inode_t));
  // find a closed entry in oft
  int oft_entry_idx = -1;
  for(int i=0; i<NUM_FD; ++i) {
    if(oft[i].state == FSTATE_CLOSED) {
      oft_entry_idx = i; 
      debug_printf(DEBUG_FILE, "\t\tFound closed entry at index %d\n", i);
      break;
    }
  }
  // If no oft records are FSTATE_CLOSED return -1
  if(oft_entry_idx == -1){
    debug_printf(DEBUG_FILE, "No closed oft records\n");
    return -1;
  }
  // fill a FSTATE_CLOSED entry in the oft with the file's metadata
  oft[oft_entry_idx].state = FSTATE_OPEN;
  oft[oft_entry_idx].head = 0;
  oft[oft_entry_idx].direntry = dirent_idx;
  oft[oft_entry_idx].inode = *inode;

  // return the index of the oft record representing the opened file
  return oft_entry_idx;
}
 