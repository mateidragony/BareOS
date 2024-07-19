#include <barelib.h>
#include <interrupts.h>
#include <bareio.h>
#include <shell.h>
#include <thread.h>
#include <queue.h>
#include <semaphore.h>
#include <malloc.h>
#include <tty.h>
#include <syscall.h>
#include <fs.h>

/*
 *  This file contains the C code entry point executed by the kernel.
 *  It is called by the bootstrap sequence once the hardware is configured.
 *      (see bootstrap.s)
 */

extern uint32* text_start;    /*                                             */
extern uint32* data_start;    /*  These variables automatically contain the  */
extern uint32* bss_start;     /*  lowest  address of important  segments in  */
extern uint32* mem_start;     /*  memory  created  when  the  kernel  boots  */
extern uint32* mem_end;       /*    (see mmap.c and kernel.ld for source)    */

void ctxload(uint64**);
void __noop(void);

uint32 boot_complete = 0;


void init_shell(void){
  printf("Kernel start: %x\n", text_start);
  printf("--Kernel size: %d\n", data_start - text_start);
  printf("Globals start: %x\n", data_start);
  printf("Heap/Stack start: %x\n", mem_start);
  printf("--Free Memory Available: %d\n", (int)(mem_end - mem_start));

  fs_create("test.txt");
  fs_create("calvin.c");
  fs_create("ClickerHero.txt");

  fs_open("test.txt");
  fs_open("calvin.c");
  fs_open("ClickerHero.txt");


  uint32 tid = create_thread(&shell, "", 1);
  resume_thread(tid);
  join_thread(tid);
}


void start(void){
  uint32 tid = create_thread(&init_shell, "", 1);
  resume_thread(tid);
  while(true) raise_syscall(RESCHED);
}

void initialize(void) {
  char mask;
  mask = disable_interrupts();
  uart_init();

  // init sem table
  for(int i=0; i<NSEM; ++i){
    semaphore_t sem;

    sem.state = S_FREE;
    sem.count = 0;
    sem.qhead = i;
    sem.qtail = i;

    sem_table[i] = sem;
  }

  // init thread table
  for(int i=0; i<NTHREADS+1; ++i){
    thread_t thread;

    thread.state = TH_FREE;
    thread.stackptr = (uint64*)get_stack(i);      
    thread.parent = 0;        
    thread.retval = 0;           
    thread.priority = -1; // MAX_UNIT32 
    thread.sem = sem_create(0);

    thread_table[i] = thread;       
  }

  // init queues
  thread_queue[ready_list].key = HEAD_PRIO;
  thread_queue[ready_list].qprev = ready_list;
  thread_queue[ready_list].qnext = ready_list;

  thread_queue[sleep_list].key = 0;
  thread_queue[sleep_list].qprev = sleep_list;
  thread_queue[sleep_list].qnext = sleep_list;

  heap_init();

  tty_init();

  bs_mk_ramdisk(NULL, NULL);
  fs_mkfs();
  fs_mount();

  boot_complete = 1;
  restore_interrupts(mask);

  if(RUNNING_AG_TESTS){
    printf("Kernel start: %x\n", text_start);
    printf("--Kernel size: %d\n", data_start - text_start);
    printf("Globals start: %x\n", data_start);
    printf("Heap/Stack start: %x\n", mem_start);
    printf("--Free Memory Available: %d\n", (int)(mem_end - mem_start));
  }


  uint32 tid = create_thread(&shell, "", 1);
  thread_table[tid].state = TH_RUNNING;
  ctxload(&(thread_table[tid].stackptr));
} 