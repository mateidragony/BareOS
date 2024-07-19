#include <barelib.h>
#include <fs.h>
#include <string.h>
#include <bareio.h>

extern fsystem_t* fsd;

/*  Search for 'filename' in the root directory.  If the  *
 *  file exists,  returns an error.  Otherwise, create a  *
 *  new file  entry in the  root directory, allocate an   *
 *  unused  block in the block  device and  assign it to  *
 *  the new file.                                         */
int32 fs_create(char* filename) {
  directory_t *root = &(fsd->root_dir);
  if(root->numentries == DIR_SIZE) return -1;

  debug_printf(DEBUG_FILE, "\n");
  debug_printf(DEBUG_FILE, "Creating file: %s\n", filename);

  bool found_entry = false;
  int free_entry = -1;
  for(int i=0; i<DIR_SIZE; ++i){
    dirent_t dirent = root->entry[i];

    if(strcmp(dirent.name, filename) == 0){
      debug_printf(DEBUG_FILE, "\tDuplicate file name!! %s == %s\n", dirent.name, filename);
      return -1; // duplicate filename
    }
    else if(dirent.inode_block == EMPTY && !found_entry){
      free_entry = i;
      found_entry = true;
    }
  }
  if(!found_entry) return -1;

  debug_printf(DEBUG_FILE, "\tfound entry: %d\n", free_entry);

  int free_block = 0;
  while(fs_getmaskbit(free_block)) free_block++;
  if(free_block == MDEV_NUM_BLOCKS) return -1;
  fs_setmaskbit(free_block);
  debug_printf(DEBUG_FILE, "\tfound free block: %d\n", free_block);

  dirent_t *entry = &root->entry[free_entry];
  root->numentries++;
  entry->inode_block = free_block;
  strncpy(entry->name, filename, FILENAME_LEN);

  inode_t inode;
  inode.id = free_block;
  inode.size = 0;
  for(int i=0; i<INODE_BLOCKS; ++i) inode.blocks[1] = EMPTY;
  // inode.blocks[0] = free_block;

  debug_printf(DEBUG_FILE, "Filename: %s\n", filename);

  bs_write(free_block, 0, (void*)&inode, sizeof(inode_t));
  bs_write(SB_BIT, 0, fsd, sizeof(fsystem_t));
  bs_write(BM_BIT, 0, fsd->freemask, fsd->freemasksz); 

  return 0;
}
 