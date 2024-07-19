#include <barelib.h>
#include <bareio.h>

extern uint32* text_start;    /*                                             */
extern uint32* data_start;    /*  These variables automatically contain the  */
extern uint32* bss_start;     /*  lowest  address of important  segments in  */
extern uint32* mem_start;     /*  memory  created  when  the  kernel  boots  */
extern uint32* mem_end;       /*    (see mmap.c and kernel.ld for source)    */

char* __ms1_general[2];
char* __ms1_printf[4];
char* __ms1_init[5];

extern char (*sys_putc_hook)(char);
extern char (*oldhook_in)(void);
extern char (*oldhook_out)(char);
extern int __ms_stdout_search;
extern int __ms_stdout_expects;
extern int __ms_stdin_expects;
extern int _ms_recovery_status;
char putc_tester(char);
void _psudoprint(const char*);
char* _copy_dec(char*, int);
char* _copy_hex(char*, unsigned int);
char* _copy_string(char*, const char*);
void __clear_string(void);
char __test_string(const char*, int, int);
char __test_template(const char*);

int _ms_safe_call(void*, ...);

void initialize(void);

char* __ms1_general_tests[] = {
			    "\n  Program compiles:                             \0",
			    "\n  `printf` is callable:                         \0"
};
char* __ms1_printf_tests[] = {
			    "\n  Calls `uart_putc`:                            \0",
			    "\n  Text passed to `uart_putc`:                   \0",
			    "\n  Decimal '%d' template prints int:             \0",
			    "\n  Hexidecimal '%x' template prints hex:         \0",
};
char* __ms1_init_tests[] = {
			    "\n  Prints correct Text start:                    \0",
			    "\n  Prints correct Kernel size:                   \0",
			    "\n  Prints correct Data start:                    \0",
			    "\n  Prints correct Memory start:                  \0",
			    "\n  Prints correct Memory size:                   \0"
};

void __ms1(void) {
  char init_text[1024];
  uart_init();
  oldhook_out = sys_putc_hook;
  sys_putc_hook = putc_tester;

  for (int i=0; i<2; i++) __ms1_general[i] = "OK\0";
  for (int i=0; i<4; i++) __ms1_printf[i]  = "OK\0";
  for (int i=0; i<5; i++) __ms1_init[i]    = "OK\0";
  for (int i=0; i<1024; i++) init_text[i] = 0;

  /* Build initialize test text */
  char* ptr = init_text;
  ptr = _copy_string(ptr, "Kernel start: 0x");
  ptr = _copy_hex(ptr, (unsigned long)text_start);
  ptr = _copy_string(ptr, "\n--Kernel size: ");
  ptr = _copy_dec(ptr, (unsigned long)(data_start - text_start));
  ptr = _copy_string(ptr, "\nGlobals start: 0x");
  ptr = _copy_hex(ptr, (unsigned long)data_start);
  ptr = _copy_string(ptr, "\nHeap/Stack start: 0x");
  ptr = _copy_hex(ptr, (unsigned long)mem_start);
  ptr = _copy_string(ptr, "\n--Free Memory Available: ");
  ptr = _copy_dec(ptr, (unsigned long)(mem_end - mem_start));

  // printf test 1
  __ms_stdout_expects = 5;
  __clear_string();
  _ms_safe_call((void (*)(void))printf, "basic");
  if (_ms_recovery_status)
    __ms1_general[1] = "FAIL - `printf` function never returns\0";
  if (__test_string("\0", 1, 0))
    __ms1_printf[0] = "FAIL - `uart_putc` not called\0";
  if (!__test_string("b", 1, 0))
    __ms1_printf[1] = "FAIL - First character sent to `uart_putc` wrong value\0";
  else if (!__test_string("basic", 5, 1))
    __ms1_printf[1] = "FAIL - Characters sent to `uart_putc` differs from string\0";

  // printf test 2
  __ms_stdout_expects = 13;
  __clear_string();
  _ms_safe_call((void (*)(void))printf, "int: %d feet\n", 12);
  if (_ms_recovery_status)
    __ms1_printf[2] = "FAIL - Printed too many characters\0";
  else if (!__test_string("int: 12 feet\n", 13, 1))
    __ms1_printf[2] = "FAIL - Printed value doesn't match expected integer value\0";

  // printf test 3
  __ms_stdout_expects = 19;
  __clear_string();
  _ms_safe_call((void (*)(void))printf, "hex: %x feet\n\n", 0x2ab30);
  if (_ms_recovery_status) {
    __ms1_printf[3] = "FAIL - Printed too many characters\0";
  }
  else if (!__test_string("hex: 0x2ab30 feet\n\n", 19, 0)) {
    if (!__test_string("hex: 2ab30 feet\n\n", 17, 0)) {
      if (!__test_string("hex: 2ab30 feet\n", 16, 0) && !__test_string("hex: 0x2ab30 feet\n", 18, 0)) {
	__ms1_printf[3] = "FAIL - Printed value doesn't match expected hexidecimal value\0";
      }
      else {
	__ms1_printf[3] = "FAIL - Did not print extra newline characters\0";
      }
    }
    else {
      __ms1_printf[3] = "FAIL - Missing '0x' in front of hex value\0";
    }
  }

  _psudoprint("\nGeneral Tests:\0");
  for (int i=0; i<2; i++) {
    _psudoprint(__ms1_general_tests[i]);
    _psudoprint(__ms1_general[i]);
  }
  _psudoprint("\n\nPrintf Tests:\0");
  for (int i=0; i<4; i++) {
    _psudoprint(__ms1_printf_tests[i]);
    _psudoprint(__ms1_printf[i]);
  }

  //  Init tests
  _psudoprint("\n\nInitialize tests:\0");
  _psudoprint("\n  * The following tests can fail by blocking and never print\n  * Less than 5 results should be considered a test failure\n\0");

  __ms_stdout_search = '\n';
  __ms_stdout_expects = 0;
  __clear_string();
  _ms_safe_call((void (*)(void))initialize);
  if (!__test_string(init_text, 25, 1))
    __ms1_init[0] = "FAIL - Kernel start line does not match expected value\0";

  _psudoprint(__ms1_init_tests[0]);
  _psudoprint(__ms1_init[0]);

  __ms_stdout_search = '\n';
  __ms_stdout_expects = 1;
  __clear_string();
  _ms_safe_call((void (*)(void))initialize);
  if (!__test_string(init_text, 45, 1))
    __ms1_init[1] = "FAIL - Kernel size line does not match expected value\0";
  
  _psudoprint(__ms1_init_tests[1]);
  _psudoprint(__ms1_init[1]);
  
  __ms_stdout_search = '\n';
  __ms_stdout_expects = 2;
  __clear_string();
  _ms_safe_call((void (*)(void))initialize);
  if (!__test_string(init_text, 71, 1))
    __ms1_init[2] = "FAIL - Globals line does not match expected value\0";
  
  _psudoprint(__ms1_init_tests[2]);
  _psudoprint(__ms1_init[2]);

  __ms_stdout_search = '\n';
  __ms_stdout_expects = 3;
  __clear_string();
  _ms_safe_call((void (*)(void))initialize);
  if (!__test_string(init_text, 100, 1))
    __ms1_init[3] = "FAIL - Memory line does not match expected value\0";
  
  _psudoprint(__ms1_init_tests[3]);
  _psudoprint(__ms1_init[3]);

  __ms_stdout_search = '\n';
  __ms_stdout_expects = 4;
  __clear_string();
  _ms_safe_call((void (*)(void))initialize);
  if (!__test_string(init_text, 133, 1))
    __ms1_init[4] = "FAIL - Total Memory size line does not match expected value\0";
  
  _psudoprint(__ms1_init_tests[4]);
  _psudoprint(__ms1_init[4]);
  
  _psudoprint("\n\n");
}
