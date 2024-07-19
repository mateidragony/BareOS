#include <queue.h>
#include <bareio.h>

/*  Queues in bareOS are all contained in the 'thread_queue' array.  Each queue has a "root"
 *  that contains the index of the first and last elements in that respective queue.  These
 *  roots are  found at the end  of the 'thread_queue'  array.  Following the 'qnext' index 
 *  of each element, starting at the "root" should always eventually lead back to the "root".
 *  The same should be true in reverse using 'qprev'.                                          */

queue_t thread_queue[NTHREADS + 2]; /*  Array of queue elements, one per thread plus one for the read_queue root  */
uint32 ready_list = NTHREADS + 0;   /*  Index of the read_list root  */
uint32 sleep_list = NTHREADS + 1;   /*  Index of the sleep_list root */


/*  'thread_enqueue' takes an index into the thread_queue  associated with a queue "root"  *
 *  and a threadid of a thread to add to the queue.  The thread will be added to the tail  *
 *  of the queue,  ensuring that the  previous tail of the queue is correctly threaded to  *
 *  maintain the queue.                                                                    */
void thread_enqueue(uint32 queue, uint32 threadid){
  queue_t *curq = &thread_queue[queue];
  uint32 curq_idx = queue;
  thread_t *cur_thread = &thread_table[threadid];

  while(curq->key <= cur_thread->priority){
    if(curq->qnext == threadid) return; // inserting existing thread

    if(curq->qnext == queue){           // end of q
      curq_idx = queue;                 // udpate curq_idx
      curq = &thread_queue[queue];      // curq is now q list
      break;                            // break to prevent infinite loop
    }
    curq_idx = curq->qnext;             // update curq idx
    curq = &thread_queue[curq_idx];     // next q
  }

  queue_t *threadq = &thread_queue[threadid];         // ---             
  threadq->key = cur_thread->priority;                //   |  Add thread queue 
  threadq->qprev = curq->qprev;                       //   |  to table
  threadq->qnext = curq_idx;                          // ---

  curq->qprev = threadid;
  thread_queue[threadq->qprev].qnext = threadid;
}


/*  'thread_dequeue' takes a queue index associated with a queue "root" and removes the  *
 *  thread at the head of the queue and returns the index of that thread, ensuring that  *
 *  the queue  maintains its structure and the head correctly points to the next thread  *
 *  (if any).                                                                            */
uint32 thread_dequeue(uint32 queue) {
  queue_t *cur_queue = &thread_queue[queue];

  if(cur_queue->qnext == queue) return NTHREADS; // no thread in list

  uint32 ret = cur_queue->qnext;                      // return head
  cur_queue->qnext = thread_queue[ret].qnext;         // ready_list next = head next
  thread_queue[cur_queue->qnext].qprev = queue;  // head next prev = readu_list

  return ret;
}


/**
 * Print the queue for debugging purposes
*/
void print_queue(uint32 queue){
  uint32 curq_idx = queue;
  queue_t *curq = &thread_queue[curq_idx];
  do{
    debug_printf(DEBUG_SLEEP, "\t\tCur %d, Prev %d, Next %d\n", curq_idx, curq->qprev, curq->qnext);
    curq_idx = curq->qnext;
    curq = &thread_queue[curq_idx];
  } while(curq_idx != queue);
}