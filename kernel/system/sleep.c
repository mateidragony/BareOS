#include <interrupts.h>
#include <queue.h>
#include <syscall.h>
#include <barelib.h>
#include <bareio.h>


/**
 * Remove a thread from a specified queue
*/
void remove_thread(uint32 qhead, uint32 threadid){
  queue_t *curq = &thread_queue[qhead];
  while(curq->qnext != threadid){
    if(curq->qnext == qhead) return; // thread not in queue
    curq = &thread_queue[curq->qnext];    // next q
  }
  curq = &thread_queue[curq->qnext]; // curq not equals thread
  // set pointers
  thread_queue[curq->qnext].qprev = curq->qprev;
  thread_queue[curq->qprev].qnext = curq->qnext;
}

void sleep_enqueue(uint32 threadid, uint32 delay){
  uint32 curq_idx = sleep_list;
  queue_t *curq = &thread_queue[curq_idx];
  while(delay >= curq->key){
    delay -= curq->key;                 // delta that delay queen
    if(curq->qnext == sleep_list){      // end of eep q
      curq_idx = sleep_list;            // udpate curq_idx
      curq = &thread_queue[curq_idx];   // curq is now sleep list
      break;                            // break to prevent infinite loop
    }
    curq_idx = curq->qnext;             // update curq idx
    curq = &thread_queue[curq_idx];     // next q
  }
  curq->key = (curq->key == 0 ? 0 : curq->key-delay); // decrease delta time of curq if it isn't already 0 (sleep_list)
  /* i now want to insert right behind curq */
  queue_t *threadq = &thread_queue[threadid];         // ---             
  threadq->key = delay;                               //   |  Add thread queue 
  threadq->qprev = curq->qprev;                       //   |  to table
  threadq->qnext = curq_idx;                          // ---

  debug_printf(DEBUG_SLEEP, "\t\t\tThreadq: prev(%d), next(%d)", threadq->qprev, threadq->qnext);

  curq->qprev = threadid;
  thread_queue[threadq->qprev].qnext = threadid;
}




/*  Places the thread into a sleep state and inserts it into the  *
 *  sleep delta list.                                             */
int32 sleep(uint32 threadid, uint32 delay) {
  debug_printf(DEBUG_SLEEP, "sleep(%d, %d)\n", threadid, delay);
  char mask;
  mask = disable_interrupts();

  if(delay == 0){
    restore_interrupts(mask);
    return raise_syscall(RESCHED);
  }

  // dq thread
  remove_thread(ready_list, threadid); // remove from ready list
  remove_thread(sleep_list, threadid); // remove from sleep list
  debug_printf(DEBUG_SLEEP, "\tDequeued thread\n");
  // nq to sleep
  sleep_enqueue(threadid, delay);
  debug_printf(DEBUG_SLEEP, "\tEnqueued thread to sleepq\n");
  // set state to sleep
  thread_table[threadid].state = TH_SLEEP;
  debug_printf(DEBUG_SLEEP, "\tSet state\n");

  print_queue(sleep_list);

  raise_syscall(RESCHED);
  debug_printf(DEBUG_SLEEP, "\tRescheduled %d\n", current_thread);

  restore_interrupts(mask);

  // resched
  return 0;
}

/*  If the thread is in the sleep state, remove the thread from the  *
 *  sleep queue and resumes it.                                      */
int32 unsleep(uint32 threadid) {
  char mask;
  mask = disable_interrupts();

  queue_t *curq = &thread_queue[sleep_list];
  while(curq->qnext != threadid){
    if(curq->qnext == sleep_list) return -1;   // thread not in queue
    curq = &thread_queue[curq->qnext];         // next q
  }
  curq = &thread_queue[curq->qnext]; // curq not equals thread

  thread_queue[curq->qnext].key += curq->key;    // update key
  thread_queue[curq->qnext].qprev = curq->qprev; // set prev ptr
  thread_queue[curq->qprev].qnext = curq->qnext; // set next ptr

  thread_table[threadid].state = TH_READY;       // set state to ready from sleep
  thread_enqueue(ready_list, threadid);          // enq thread

  raise_syscall(RESCHED);

  restore_interrupts(mask);
  return 0;
}
