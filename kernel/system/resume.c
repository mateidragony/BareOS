#include <barelib.h>
#include <interrupts.h>
#include <syscall.h>
#include <thread.h>
#include <queue.h>

/*  Takes a index into the thread table of a thread to resume.  If the thread is already  *
 *  ready  or running,  returns an error.  Otherwise, adds the thread to the ready list,  *
 *  sets  the thread's  state to  ready and raises a RESCHED  syscall to  schedule a new  *
 *  thread.  Returns the threadid to confirm resumption.                                  */
int32 resume_thread(uint32 threadid) {
  char mask;
  mask = disable_interrupts();

  thread_t *thread = &thread_table[threadid];
  if(thread->state != TH_SUSPEND){
    restore_interrupts(mask);
    return EXIT_FAILURE;
  }

  thread->state = TH_READY;
  thread_enqueue(ready_list, threadid);
  raise_syscall(RESCHED);

  restore_interrupts(mask);
  return threadid;
}

