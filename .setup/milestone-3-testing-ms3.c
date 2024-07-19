#include <barelib.h>
#include <bareio.h>
#include <thread.h>
#include <syscall.h>

char* __ms3_general[1];
char* __ms3_suspend[3];
char* __ms3_resume[3];
char* __ms3_join[3];
char* __ms3_resched[5];
char* __ms3_shell[5];

char* __ms3_general_tests[] = {
			    "\n  Program Compiles:                  \0"
};
char* __ms3_suspend_tests[] = {
			    "\n  Indicated thread was suspended:    \0",
			    "\n  Invalid threads not suspended:     \0",
			    "\n  Raises RESCHED signal:             \0",
};
char* __ms3_resume_tests[] = {
			    "\n  Indicated thread was readied:      \0",
			    "\n  Invalid threads are ignored:       \0",
			    "\n  Raises RESCHED signal:             \0"
};
char* __ms3_join_tests[] = {
			    "\n  Frees awaited thread:              \0",
			    "\n  Returns the thread's return value: \0",
			    "\n  Blocks until thread is defunct:    \0",
};
char* __ms3_resched_tests[] = {
			    "\n  Alternates between two threads:    \0",
			    "\n  Cycles between three threads:      \0",
			    "\n  Sets old thread as ready:          \0",
			    "\n  Sets the current thread:           \0",
			    "\n  Sets the new thread to running:    \0",
};
char* __ms3_shell_tests[] = {
			     "\n  Started with ctxload:              \0",
			     "\n  Creates applications with threads: \0",
			     "\n  Calls `create_thread`:             \0",
			     "\n  Calls `resume_thread`:             \0",
			     "\n  Calls `join_thread`:               \0",
};

extern char (*sys_putc_hook)(char);
extern char (*sys_getc_hook)(void);
extern void (*sys_syscall_hook)(void);
extern char (*oldhook_in)(void);
extern char (*oldhook_out)(char);
extern int32 _sys_thread_loaded;
extern int __ms_stdout_expects;
extern int __ms_stdin_expects;
extern int __ms_syscall_expects;
extern int _ms_recovery_status;
char putc_tester(char);
char getc_tester(void);
void syscall_tester(void);
void _psudoprint(const char*);
void _prep_stdin(const char*);
void __clear_string(void);
char __test_string(const char*, int, int);

int _ms_safe_call(void*, ...);

char initialize(char*);
void ctxload(uint64**);

void __ms3_reset_threads(void) {
  for (int i=1; i<NTHREADS; i++) thread_table[i].state = TH_FREE;
  current_thread = 0;
}

void _ms_protoresume(int32 tid) {
  thread_table[tid].state = TH_READY;
}

char __ms3_t1(char* arg) {
  return 0;
}

byte __ms3_t2_validator = 0;
char __ms3_t2(char* arg) {
  byte validator = __ms3_t2_validator;
  __ms3_t2_validator += 1;
  raise_syscall(RESCHED);
  if (__ms3_t2_validator < validator + 2)
    __ms3_resched[0] = "FAIL - Thread ran again before all other threads ran\0";
  __ms3_t2_validator += 1;
  raise_syscall(RESCHED);
  if (__ms3_t2_validator < validator + 4)
    __ms3_resched[0] = "FAIL - Thread ran again before all other threaads ran\0";
  return 0;
}

byte __ms3_t3_validator = 0;
char __ms3_t3(char* arg) {
  byte validator = __ms3_t3_validator;
  __ms3_t3_validator += 1;
  raise_syscall(RESCHED);
  if (__ms3_t3_validator < validator + 3)
    __ms3_resched[1] = "FAIL - Thread ran again before all other threads ran\0";
  __ms3_t3_validator += 1;
  raise_syscall(RESCHED);
  if (__ms3_t3_validator < validator + 6)
    __ms3_resched[1] = "FAIL - Thread ran again before all other threaads ran\0";
  __ms3_t3_validator += 1;
  return 0;
}

char __ms3_t4(char* arg) {
  switch (thread_table[0].state) {
  case TH_FREE    : __ms3_resched[2] = "FAIL - Previous thread set to FREE\0"; break;
  case TH_RUNNING : __ms3_resched[2] = "FAIL - Previous thread set to RUNNING\0"; break;
  case TH_SUSPEND : __ms3_resched[2] = "FAIL - Previous thread set to SUSPEND\0"; break;
  case TH_DEFUNCT : __ms3_resched[2] = "FAIL - Previous thread set to DEFUNCT\0"; break;
  default : break;
  }

  if (current_thread == 0)
    __ms3_resched[3] = "FAIL - Current thread is not set to the newly running thread\0";

  switch (thread_table[current_thread].state) {
  case TH_FREE    : __ms3_resched[4] = "FAIL - Current running thread set to FREE\0"; break;
  case TH_READY   : __ms3_resched[4] = "FAIL - Current running thread set to READY\0"; break;
  case TH_SUSPEND : __ms3_resched[4] = "FAIL - Current running thread set to SUSPEND\0"; break;
  case TH_DEFUNCT : __ms3_resched[4] = "FAIL - Current running thread set to DEFUNCT\0"; break;
  }
  return 0;
}

void __ms3_suspend_tester(void) {
  __ms3_reset_threads();
  /*
   * Sets called thread to suspend if valid
   * Ignores invalid threads
   * Raises RESCHED
   */
  __ms_syscall_expects = 0;
  int32 tid = create_thread(__ms3_t1, "", 0);
  thread_table[tid].state = TH_READY;
  (int)_ms_safe_call((void (*)(void))suspend_thread, tid);
  switch (thread_table[tid].state) {
  case TH_FREE    : __ms3_suspend[0] = "FAIL - Thread set to FREE after suspend\0"; break;
  case TH_RUNNING : __ms3_suspend[0] = "FAIL - Thread set to RUNNING after suspend\0"; break;
  case TH_READY   : __ms3_suspend[0] = "FAIL - Thread set to READY after suspend\0"; break;
  case TH_DEFUNCT : __ms3_suspend[0] = "FAIL - Thread set to DEFUNCT after suspend\0"; break;
  default : break;
  }
  if (_ms_recovery_status != 3) {
    __ms3_suspend[2] = "FAIL - RESCHED system call was not raised\0";
  }
  
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_FREE;
  (int)_ms_safe_call((void (*)(void))suspend_thread, tid);
  if (_ms_recovery_status == 3) {
    __ms3_suspend[1] = "FAIL - RESCHED called when thread was FREE\0";
  }
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_SUSPEND;
  (int)_ms_safe_call((void (*)(void))suspend_thread, tid);
  if (_ms_recovery_status == 3) {
    __ms3_suspend[1] = "FAIL - RESCHED called when thread was SUSPEND\0";
  }
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_DEFUNCT;
  (int)_ms_safe_call((void (*)(void))suspend_thread, tid);
  if (_ms_recovery_status == 3) {
    __ms3_suspend[1] = "FAIL - RESCHED called when thread was DEFUNCT\0";
  }
}

void __ms3_resume_tester(void) {
  __ms3_reset_threads();
  /*
   * Sets called thread to TH_READY if suspended
   * Ignores invalid threads
   * Raises RESCHED
   */
  __ms_syscall_expects = 0;
  int32 tid = create_thread(__ms3_t1, "", 0);
  (int)_ms_safe_call((void (*)(void))resume_thread, tid);
  switch (thread_table[tid].state) {
  case TH_FREE    : __ms3_resume[0] = "FAIL - Thread set to FREE after resume\0"; break;
  case TH_RUNNING : __ms3_resume[0] = "FAIL - Thread set to RUNNING after resume\0"; break;
  case TH_SUSPEND : __ms3_resume[0] = "FAIL - Thread set to SUSPEND after resume\0"; break;
  case TH_DEFUNCT : __ms3_resume[0] = "FAIL - Thread set to DEFUNCT after suspend\0"; break;
  default : break;
  }
  if (_ms_recovery_status != 3) {
    __ms3_resume[2] = "FAIL - RESCHED system call was not raised\0";
  }
  
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_FREE;
  (int)_ms_safe_call((void (*)(void))resume_thread, tid);
  if (_ms_recovery_status == 3) {
    __ms3_resume[1] = "FAIL - RESCHED called when thread was FREE\0";
  }
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_RUNNING;
  (int)_ms_safe_call((void (*)(void))resume_thread, tid);
  if (_ms_recovery_status == 3) {
    __ms3_resume[1] = "FAIL - RESCHED called when thread was RUNNING\0";
  }
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_READY;
  (int)_ms_safe_call((void (*)(void))resume_thread, tid);
  if (_ms_recovery_status == 3) {
    __ms3_resume[1] = "FAIL - RESCHED called when thread was READY\0";
  }
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_DEFUNCT;
  (int)_ms_safe_call((void (*)(void))resume_thread, tid);
  if (_ms_recovery_status == 3) {
    __ms3_resume[1] = "FAIL - RESCHED called when thread was DEFUNCT\0";
  }
}

void __ms3_join_tester(void) {
  int result;
  __ms3_reset_threads();
  /*
   * Sets called thread to TH_FREE if TH_DEFUNCT
   * Returns the processes retval
   * Rescheds while thread is not TH_DEFUNCT
   */
  __ms_syscall_expects = 0;
  int32 tid = create_thread(__ms3_t1, "", 0);
  thread_table[tid].state = TH_DEFUNCT;
  thread_table[tid].retval = 12;
  result = (int)_ms_safe_call((void (*)(void))join_thread, tid);
  if (_ms_recovery_status == 3)
    __ms3_join[0] = "FAIL - RESCHED raised when thread was DEFUNCT\0";
  else if (thread_table[tid].state == TH_RUNNING)
    __ms3_join[0] = "FAIL - Thread state set to RUNNING after join\0";
  else if (thread_table[tid].state == TH_READY)
    __ms3_join[0] = "FAIL - Thread state set to READY after join\0";
  else if (thread_table[tid].state == TH_SUSPEND)
    __ms3_join[0] = "FAIL - Thread state set to SUSPEND after join\0";
  else if (thread_table[tid].state == TH_DEFUNCT)
    __ms3_join[0] = "FAIL - Thread state set to DEFUNCT after join\0";

  if (result != 12)
    __ms3_join[1] = "FAIL - `join_thread` did not return thread result\0";

  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_READY;
  (int)_ms_safe_call((void (*)(void))join_thread, tid);
  if (_ms_recovery_status != 3)
    __ms3_join[2] = "FAIL - REACHED not raised when thread was READY\0";
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_FREE;
  (int)_ms_safe_call((void (*)(void))join_thread, tid);
  if (_ms_recovery_status == 3)
    __ms3_join[2] = "FAIL - REACHED raised when thread was FREE\0";
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_RUNNING;
  (int)_ms_safe_call((void (*)(void))join_thread, tid);
  if (_ms_recovery_status != 3)
    __ms3_join[2] = "FAIL - REACHED not raised when thread was RUNNING\0";
  __ms_syscall_expects = 0;
  thread_table[tid].state = TH_SUSPEND;
  (int)_ms_safe_call((void (*)(void))join_thread, tid);
  if (_ms_recovery_status != 3)
    __ms3_join[2] = "FAIL - REACHED not raised when thread was SUSPEND\0";
}

void __ms3_resched_tester(void) {
  thread_table[0].state = TH_RUNNING;
  __ms3_reset_threads();
  /*
   * Correctly swaps two threads
   * Correctly cycles three threads
   * Sets previous thread state to TH_READY
   * Sets the current_thread
   * Sets the new thread state to TH_RUNNING
   */
  __ms_syscall_expects = -1;
  int32 tid1 = create_thread(__ms3_t4, "", 0);
  _ms_protoresume(tid1);
  raise_syscall(RESCHED);

  __ms3_reset_threads();
  tid1 = create_thread(__ms3_t2, "", 0);
  _ms_protoresume(tid1);
  raise_syscall(RESCHED);
  __ms3_t2_validator += 1;
  raise_syscall(RESCHED);
  __ms3_t2_validator += 1;
  raise_syscall(RESCHED);

  __ms3_reset_threads();
  tid1 = create_thread(__ms3_t3, "", 0);
  int32 tid2 = create_thread(__ms3_t3, "", 0);
  _ms_protoresume(tid1);
  _ms_protoresume(tid2);
  raise_syscall(RESCHED);
  __ms3_t3_validator += 1;
  raise_syscall(RESCHED);
  __ms3_t3_validator += 1;
  raise_syscall(RESCHED);
}

void __ms3_shell_tester(void) {
  __ms3_reset_threads();
  /*
   * Initializes with ctxload
   * Creates applications as threads
   * Calls create_thread
   * Calls resume_thread
   * Calls join_thread
   */

  __ms_stdin_expects = 0;
  __ms_syscall_expects = 0;
  _sys_thread_loaded = 0;
  _prep_stdin("\0");
  (int)_ms_safe_call((void (*)(void))initialize);
  if (!_sys_thread_loaded)
    __ms3_shell[0] = "FAIL - `ctxload` not called\0";

  __ms_stdin_expects = 8;
  _prep_stdin("hello v\n\0");
  __ms_syscall_expects = 1;
  (int)_ms_safe_call((void (*)(void))initialize);
  if (_ms_recovery_status != 3)
    __ms3_shell[1] = "FAIL - RESCHED not called when starting application\0";
  __ms3_shell[2] = "N/A - Test could not be automated\0";
  __ms3_shell[3] = "N/A - Test could not be automated\0";
  __ms3_shell[4] = "N/A - Test could not be automated\0";

  _psudoprint("\nShell Tests:\0");
  for (int i=0; i<5; i++) {
    _psudoprint(__ms3_shell_tests[i]);
    _psudoprint(__ms3_shell[i]);
  }

  _psudoprint("\n\n");
  asm volatile (
		"__loop:\n"
                    "wfi\n"
                    "j __loop\n"
		);
}

char __ms3_runner(char* arg) {
  int i;
  for (i=0; i<1; i++) __ms3_general[i] = "OK\0";
  for (i=0; i<3; i++) __ms3_suspend[i] = "OK\0";
  for (i=0; i<3; i++) __ms3_resume[i]  = "OK\0";
  for (i=0; i<3; i++) __ms3_join[i]    = "OK\0";
  for (i=0; i<5; i++) __ms3_resched[i] = "OK\0";
  for (i=0; i<5; i++) __ms3_shell[i]   = "OK\0";
  
  _psudoprint("\n  * Expected test count: 20\n  *     Less than 20 results should be considered a test failure\n\0");

  _psudoprint("\nGeneral Tests:\0");
  for (i=0; i<1; i++) {
    _psudoprint(__ms3_general_tests[i]);
    _psudoprint(__ms3_general[i]);
  }
  
  __ms3_suspend_tester();
  _psudoprint("\nSuspend Tests:\0");
  for (i=0; i<3; i++) {
    _psudoprint(__ms3_suspend_tests[i]);
    _psudoprint(__ms3_suspend[i]);
  }
  __ms3_resume_tester();
  _psudoprint("\nResume Tests:\0");
  for (i=0; i<3; i++) {
    _psudoprint(__ms3_resume_tests[i]);
    _psudoprint(__ms3_resume[i]);
  }

  __ms3_join_tester();
  _psudoprint("\nJoin Tests:\0");
  for (i=0; i<3; i++) {
    _psudoprint(__ms3_join_tests[i]);
    _psudoprint(__ms3_join[i]);
  }

  __ms3_resched_tester();
  _psudoprint("\nResched Tests:\0");
  for (i=0; i<5; i++) {
    _psudoprint(__ms3_resched_tests[i]);
    _psudoprint(__ms3_resched[i]);
  }

  __ms3_shell_tester();
  _psudoprint("\nShell Tests:\0");
  for (i=0; i<5; i++) {
    _psudoprint(__ms3_shell_tests[i]);
    _psudoprint(__ms3_shell[i]);
  }

  _psudoprint("\n\n");
  return 0;
}

void __ms3(void) {
  thread_table[0].state = TH_FREE;
  uart_init();
  oldhook_out = sys_putc_hook;
  oldhook_in = sys_getc_hook;
  __clear_string();
  sys_putc_hook = putc_tester;
  sys_getc_hook = getc_tester;
  sys_syscall_hook = syscall_tester;
    
  int32 tid = create_thread(__ms3_runner, "", 0);
  ctxload(&(thread_table[tid].stackptr));
}
