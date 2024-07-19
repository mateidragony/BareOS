#include <interrupts.h>
#include <queue.h>
#include <semaphore.h>
#include <syscall.h>

#include <bareio.h>

semaphore_t sem_table[NSEM];

/*  Finds an unusued semaphore in the sem_table and returns it after  *
 *  resetting its values and marking it as used.                      */
int32 sem_create(int32 count) {
  char mask = disable_interrupts();

  int32 sem_idx = -1;
  for(int i=0; i!=NSEM; ++i){
    if(sem_table[i].state == S_FREE){
      sem_idx = i;
      break;
    }
  }
  if(sem_idx != -1){
    sem_table[sem_idx].state = S_USED;
    sem_table[sem_idx].count = count;
  }

  restore_interrupts(mask);
  return sem_idx;
}

/*  Marks a semaphore as free  */
int32 sem_free(uint32 sid) {
  if(sem_table[sid].state == S_FREE) return -1;
  
  uint32 cur_th = sem_table[sid].qhead;
  while(cur_th != sid){
    thread_table[cur_th].state = TH_READY;
    thread_enqueue(ready_list, cur_th);
    cur_th = thread_queue[cur_th].qnext;
  }

  sem_table[sid].state = S_FREE;
  return 0;
}

/*  Decrements the given semaphore if it is in use.  If the semaphore  *
 *  is less than 0, marks the thread as waiting and switches to        *
 *  another thread.                                                    */
int32 sem_wait(uint32 sid) {
  char mask;
  mask = disable_interrupts();
  // If semaphore is invalid or free, function MUST return -1
  if(sid >= NSEM) return -1;
  semaphore_t *sem = &sem_table[sid];
  if(sem->state == S_FREE) return -1;

  if((--(sem->count)) < 0){
    // place thread onto queue
    queue_t *curq = &thread_queue[current_thread];
    curq->qnext = sid;
    curq->qprev = sem->qtail;

    sem->qtail = current_thread;
    if(sem->qhead == sid) sem->qhead = current_thread;
    else thread_queue[sem->qtail].qnext = current_thread;
    // wait thread
    thread_table[current_thread].state = TH_WATING;
    // resched
    restore_interrupts(mask);
    raise_syscall(RESCHED);
  }
  
  restore_interrupts(mask);
  return 0;
}

/*  Increments the given semaphore if it is in use.  Resume the next  *
 *  waiting thread (if any).                                          */
int32 sem_post(uint32 sid) {
  char mask;
  mask = disable_interrupts();
  // If semaphore is invalid or free, function MUST return -1
  if(sid >= NSEM) return -1;
  semaphore_t *sem = &sem_table[sid];
  if(sem->state == S_FREE) return -1;

  if((++(sem->count)) >= 0 && sem->qhead != sid){
    // get first thread in sem q
    uint32 th = sem->qhead;
    // dq thread from sem q
    thread_queue[sem->qhead].qprev = sid;
    sem->qhead = thread_queue[sem->qhead].qnext;
    // enq thread to ready list
    thread_enqueue(ready_list, th);
    // resched
    restore_interrupts(mask);
    raise_syscall(RESCHED);
  }
  

  restore_interrupts(mask);
  return 0;
}



static int32 sem1, sem2;
static int num = 5;

void sem_foo(void) { 
  printf("In %d ticks ", num);
  for (int i=0; i<num; i++) {
    sem_post(sem1);
    sem_wait(sem2);
    printf(".");
  }
}
void sem_bar(void) { 
  for (int i=0; i<num; i++) {
    sem_wait(sem1);
    printf("%d", num-i);
    sem_post(sem2);
  }
}

void sem_main(void) {
  sem1 = sem_create(0);
  sem2 = sem_create(0);

  uint32 tid1, tid2;
  tid1 = create_thread(sem_foo, NULL, 0);
  tid2 = create_thread(sem_bar, NULL, 0);
  resume_thread(tid1);
  resume_thread(tid2);
  join_thread(tid1);
  join_thread(tid2);

  sem_free(sem1);
  sem_free(sem2);
  printf("\n");
}