#include <barelib.h>
#include <fs.h>
#include <malloc.h>

extern fsystem_t* fsd;
extern filetable_t oft[NUM_FD];

/*  Modify  the state  of the open  file table to  close  *
 *  the 'fd' index and write the inode back to the block  *
    device.  If the  entry is already closed,  return an  *
 *  error.                                                */
int32 fs_close(int32 fd) {
  // If oft entry is FSTATE_CLOSED, function MUST return -1
  if(oft[fd].state == FSTATE_CLOSED) return -1;
  // Function MUST write oft inode to the ramdisk
  inode_t inode = oft[fd].inode;
  bs_write(inode.id, 0, &inode, sizeof(inode_t));
  // Function MUST close the oft entry  
  free(&(oft[fd].inode));
  oft[fd].state = FSTATE_CLOSED;
  return 0;
}
 