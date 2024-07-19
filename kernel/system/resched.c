#include <barelib.h>
#include <thread.h>
#include <interrupts.h>
#include <queue.h>

#include <bareio.h>


// fix thread priority

/*  'resched' places the current running thread into the ready state  *
 *  and  places it onto  the tail of the  ready queue.  Then it gets  *
 *  the head  of the ready  queue  and sets this  new thread  as the  *
 *  'current_thread'.  Finally,  'resched' uses 'ctxsw' to swap from  *
 *  the old thread to the new thread.                                 */
int32 resched(void){

  char mask = disable_interrupts();

  uint32 head_idx = thread_dequeue(ready_list);

  if(head_idx == NTHREADS) {
    resume_thread(current_thread);
    restore_interrupts(mask);
    return 0 ;
  }

  char cur_state = thread_table[current_thread].state;
  if(cur_state == TH_READY || cur_state == TH_RUNNING) thread_enqueue(ready_list, current_thread);
  thread_table[current_thread].state = (cur_state == TH_RUNNING ? TH_READY : cur_state);  // set current to ready if it was running
  thread_table[head_idx].state = TH_RUNNING;                                              // set head to running

  uint32 old_thread = current_thread;                                                     // remember old thread
  current_thread = head_idx;                                                              // set current_thread to head

  ctxsw(&(thread_table[current_thread].stackptr), &(thread_table[old_thread].stackptr));

  restore_interrupts(mask);

  return 0;
}




int32 _old_resched(void) {

  char mask = disable_interrupts();

  int ready_idx = 0;
  for(; ready_idx<NTHREADS; ++ready_idx) if(thread_table[(ready_idx + current_thread) % NTHREADS].state == TH_READY) goto found_ready;  
  restore_interrupts(mask);
  return 0;

  found_ready:
  int head_idx = (ready_idx + current_thread) % NTHREADS;

  char cur_state = thread_table[current_thread].state;
  thread_table[current_thread].state = (cur_state == TH_RUNNING ? TH_READY : cur_state);  // set current to ready if it was running
  thread_table[head_idx].state = TH_RUNNING;                                              // set head to running

  uint32 old_thread = current_thread;                                                     // remember old thread
  current_thread = head_idx;                                                              // set current_thread to head

  ctxsw(&(thread_table[current_thread].stackptr), &(thread_table[old_thread].stackptr));

  restore_interrupts(mask);

  return 0;
}
