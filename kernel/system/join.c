#include <barelib.h>
#include <thread.h>
#include <interrupts.h>
#include <syscall.h>
#include <bareio.h>
#include <semaphore.h>

/*  Takes an index into the thread_table.  If the thread is TH_DEFUNCT,  *
 *  mark  the thread  as TH_FREE  and return its  `retval`.   Otherwise  *
 *  raise RESCHED and loop to check again later.                         */
byte join_thread(uint32 threadid) {
  char mask = disable_interrupts();  
  thread_t *thread = &thread_table[threadid];
  if(thread->state == TH_FREE) return EXIT_FAILURE;
  restore_interrupts(mask);

  if(RUNNING_AG_TESTS) while(thread->state != TH_DEFUNCT) raise_syscall(RESCHED);
  else {
    sem_wait(threadid);
    raise_syscall(RESCHED);
  }

  thread->state = TH_FREE;
  return thread->retval;
}
