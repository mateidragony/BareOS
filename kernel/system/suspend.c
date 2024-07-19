#include <barelib.h>
#include <interrupts.h>
#include <syscall.h>
#include <thread.h>
#include <queue.h>


/*  Takes a index into the thread table of a thread to suspend.  If the thread is  *
 *  not in the  running or  ready state,  returns an error.   Otherwise, sets the  *
 *  thread's  state  to  suspended  and  raises a  RESCHED  syscall to schedule a  *
 *  different thread.  Returns the threadid to confirm suspension.                 */
int32 suspend_thread(uint32 threadid) {
  char mask;
  mask = disable_interrupts();
  
  thread_t *thread = &thread_table[threadid];
  if(thread->state != TH_READY && thread->state != TH_RUNNING){
    restore_interrupts(mask);
    return EXIT_FAILURE;
  }
  thread->state = TH_SUSPEND;
  thread_dequeue(ready_list);
  raise_syscall(RESCHED);
  
  restore_interrupts(mask);
  return threadid;
}
