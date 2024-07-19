#include <barelib.h>
#include <fs.h>

#include <bareio.h>

extern fsystem_t* fsd;
extern filetable_t oft[NUM_FD];

/* fs_write - Takes a file descriptor index into the 'oft', a  pointer to a  *
 *            buffer  that the  function reads data  from and the number of  *
 *            bytes to copy from the buffer to the file.                     *
 *                                                                           *
 *            'fs_write' reads data from the 'buff' and copies it into the   *
 *            file  'blocks' starting  at the 'head'.  The  function  will   *
 *            allocate new blocks from the block device as needed to write   *
 *            data to the file and assign them to the file's inode.          *
 *                                                                           *
 *  returns - 'fs_write' should return the number of bytes written to the    *
 *            file.                                                          */
uint32 fs_write(uint32 fd, char* buff, uint32 len) {

  debug_printf(DEBUG_FILE, "\n");
  debug_printf(DEBUG_FILE, "Writing to file: %d, len %d\n", fd, len);

  // get the stuff
  if(oft[fd].state == FSTATE_CLOSED) return -1;
  filetable_t *ft_entry = &oft[fd];
  inode_t *inode = &(ft_entry->inode);

  // calculate how many total blocks I need
  int total_blocks_needed = (len + inode->size) / MDEV_BLOCK_SIZE + ((len + inode->size) % MDEV_BLOCK_SIZE ? 1 : 0);
  if(total_blocks_needed >= INODE_BLOCKS){
    debug_printf(DEBUG_FILE, "\tNot enough room: %d\n", total_blocks_needed);
    return -1;
  }

  debug_printf(DEBUG_FILE, "\tTotal blocks needed: %d\n", total_blocks_needed);

  // calculate how many blocks I currently have
  int cur_num_blocks = inode->size / MDEV_BLOCK_SIZE + (inode->size % MDEV_BLOCK_SIZE ? 1 : 0);
  debug_printf(DEBUG_FILE, "\tCurrent number of blocks in innode: %d\n", cur_num_blocks);

  // reserve my blocks
  int free_block = 0;
  while(cur_num_blocks < total_blocks_needed){
    while(fs_getmaskbit(free_block)) free_block++;
    inode->blocks[cur_num_blocks++] = free_block;
    fs_setmaskbit(free_block);
  }

  // write data to file
  int buff_idx = 0;
  while(buff_idx < len){
    int cur_block = inode->blocks[ft_entry->head / MDEV_BLOCK_SIZE];

    bs_write(cur_block, ft_entry->head % MDEV_BLOCK_SIZE, (void *)&(buff[buff_idx++]), sizeof(char));
    (ft_entry->head)++;
  }

  // update inode and bitmask
  uint32 old_size = inode->size;
  inode->size = ft_entry->head > inode->size ? ft_entry->head : inode->size;
  bs_write(inode->id, 0, (void*)inode, sizeof(inode_t));
  bs_write(BM_BIT, 0, fsd->freemask, fsd->freemasksz); 

  return inode->size - old_size;
}
