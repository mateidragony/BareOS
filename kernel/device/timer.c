/*
 *  This file contains functions for initializing and handling interrupts
 *  from the hardware timer.
 */

#include <barelib.h>
#include <interrupts.h>
#include <queue.h>
#include <bareio.h>

#define TRAP_TIMER_ENABLE 0x80

volatile uint32* clint_timer_addr  = (uint32*)0x2004000;
//const uint32 timer_interval = 50000; // 10000
const uint32 timer_interval = 50000; // 10000
int64 resched(void);

/*
 * This function is called as part of the bootstrapping sequence
 * to enable the timer. (see bootstrap.s)
 */
void clk_init(void) {
  *clint_timer_addr = timer_interval;
  set_interrupt(TRAP_TIMER_ENABLE);
}



/**
 * Function MUST decrement the key of the head of the sleep_list
 * Function MUST dequeue while the key of the head is 0
 * Threads dequeued MUST be added to the ready_list
*/
void _handle_sleep_clk(void){
  if(thread_queue[sleep_list].qnext == sleep_list) return; // nothing in sleep queue
  debug_printf(DEBUG_SLEEP, "Clock - Something in sleepq\n");

  print_queue(sleep_list);

  uint32 curq_idx = thread_queue[sleep_list].qnext;        // q idx
  queue_t *curq = &thread_queue[curq_idx];                 // cur q ptr
  
  curq->key = (curq->key > 0 ? curq->key - 1 : 0);         // decrease key to min 0. It wouldn't go negative anyway just there for absolute protection

  while(curq->key == 0){                            // keep dqing until you reach a thread that is still sleeping
    thread_table[curq_idx].state = TH_READY;        // set state to ready from sleep

    thread_queue[curq->qprev].qnext = curq->qnext;  // dq thread from sleep q 
    thread_queue[curq->qnext].qprev = curq->qprev;  //

    uint32 next = curq->qnext;

    thread_enqueue(ready_list, curq_idx);           // enq thread

    if(next == sleep_list) break;            // at end of sleepq
    curq_idx = next;                         // move curq idx
    curq = &thread_queue[curq_idx];          // go to next q
  }
}


/* 
 * This function is triggered every 'timer_interval' microseconds 
 * automatically.  (see '__traps' in bootstrap.s)
 */
interrupt handle_clk(void) {
  *clint_timer_addr += timer_interval;
  if (boot_complete && is_interrupting()) {
    char mask = disable_interrupts();

    _handle_sleep_clk();

    resched();
    restore_interrupts(mask);
  }
}
