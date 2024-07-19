#include <barelib.h>
#include <fs.h>

#include <bareio.h>

/* fs_read - Takes a file descriptor index into the 'oft', a  pointer to a  *
 *           buffer that the function writes data to and a number of bytes  *
 *           to read.                                                       *
 *                                                                          *
 *           'fs_read' reads data starting at the open file's 'head' until  *
 *           it has  copied either 'len' bytes  from the file's  blocks or  *
 *           the 'head' reaches the end of the file.                        *
 *                                                                          *
 * returns - 'fs_read' should return the number of bytes read (either 'len' *
 *           or the  number of bytes  remaining in the file,  whichever is  *
 *           smaller).                                                      */
uint32 fs_read(uint32 fd, char* buff, uint32 len) {
  debug_printf(DEBUG_FILE, "\n");
  debug_printf(DEBUG_FILE, "Reading from file: %d, len %d\n", fd, len);

  // get the stuff
  if(oft[fd].state == FSTATE_CLOSED) return -1;
  filetable_t *ft_entry = &oft[fd];
  inode_t *inode = &(ft_entry->inode);

  // write data to file
  int buff_idx = 0;
  while(buff_idx < len && ft_entry->head < inode->size){
    int cur_block = inode->blocks[ft_entry->head / MDEV_BLOCK_SIZE];
    
    bs_read(cur_block, ft_entry->head % MDEV_BLOCK_SIZE, (void *)&(buff[buff_idx++]), sizeof(char));
    (ft_entry->head)++;
  }

  // update inode and bitmask
  // inode->size = ft_entry->head > inode->size ? ft_entry->head : inode->size;
  // bs_write(inode->id, 0, (void*)inode, sizeof(inode_t));
  // bs_write(BM_BIT, 0, fsd->freemask, fsd->freemasksz); 

  return buff_idx;
}
