#include <interrupts.h>
#include <queue.h>

/*  Places the thread into a sleep state and inserts it into the  *
 *  sleep delta list.                                             */
int32 sleep(uint32 threadid, uint32 delay) {
  char mask;
  mask = disable_interrupts();


  restore_interrupts(mask);
  return 0;
}

/*  If the thread is in the sleep state, remove the thread from the  *
 *  sleep queue and resumes it.                                      */
int32 unsleep(uint32 threadid) {
  char mask;
  mask = disable_interrupts();


  restore_interrupts(mask);
  return 0;
}
