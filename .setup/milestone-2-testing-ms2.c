#include <barelib.h>
#include <bareio.h>

char* __ms2_general[3];
char* __ms2_echo[5];
char* __ms2_hello[4];
char* __ms2_shell[6];

extern char (*sys_putc_hook)(char);
extern char (*sys_getc_hook)(void);
extern void (*sys_syscall_hook)(void);
extern char (*oldhook_in)(void);
extern char (*oldhook_out)(char);
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

char builtin_echo(char*);
char builtin_hello(char*);
char shell(char*);

char* __ms2_general_tests[] = {
		       "\n  Program Compiles:                         \0",
		       "\n  `echo` is callable:                       \0",
		       "\n  `hello` is callable:                      \0"
};
char* __ms2_echo_tests[] = {
		       "\n  Prints the argument text:                 \0",
		       "\n  Echos a line of text:                     \0",
		       "\n  Returns when encountering an empty line:  \0",
		       "\n  Returns 0 when an argument is passed in:  \0",
		       "\n  Returns the number of characters read:    \0"
};
char* __ms2_hello_tests[] = {
		       "\n  Prints error on no argument received:     \0",
		       "\n  Echos the correct string:                 \0",
		       "\n  Returns 1 on error:                       \0",
		       "\n  Returns 0 on success:                     \0"
};
char* __ms2_shell_tests[] = {
		       "\n  Prints error on bad command:              \0",
		       "\n  Reads only up to newline each command:    \0",
		       "\n  Sucessfully calls `echo`:                 \0",
		       "\n  Successfully calls `hello`:               \0",
		       "\n  Shell loops after command completes:      \0",
		       "\n  Shell replaces '$?' with previous result: \0"
};

void __ms2(void) {
  int result;
  uart_init();
  oldhook_out = sys_putc_hook;
  oldhook_in = sys_getc_hook;
  __clear_string();
  sys_putc_hook = putc_tester;
  sys_getc_hook = getc_tester;
  sys_syscall_hook = syscall_tester;
  __ms_syscall_expects = 0;

  for (int i=0; i<3; i++) __ms2_general[i] = "OK\0";
  for (int i=0; i<5; i++) __ms2_echo[i]    = "OK\0";
  for (int i=0; i<4; i++) __ms2_hello[i]   = "OK\0";
  for (int i=0; i<6; i++) __ms2_shell[i]   = "OK\0";

  _psudoprint("\n  * Expected test count: 18\n  * Less than 18 results should be considered a test failure\n\0");

  // Echo test 1
  __ms_stdout_expects = 10;
  __clear_string();
  result = (int)_ms_safe_call((void (*)(void))builtin_echo, "echo Echo test\0");
  if (_ms_recovery_status) {
    __ms2_general[1] = "FAIL - Function did not return\0";
    __ms2_echo[0] = "FAIL - 'uart_putc' Called too many times\0";
  }
  else if (!__test_string("Echo test\n", 10, 1))
    __ms2_echo[0] = "FAIL - Output text is not the same as text passed in\0";
  if (result)
    __ms2_echo[3] = "FAIL - Returned non-0 value\0";

  // Echo test 2
  __ms_stdout_expects = 17;
  __ms_stdin_expects = 17;
  __clear_string();
  _prep_stdin("Echo this string\n\0");
  result = (int)_ms_safe_call((void (*)(void))builtin_echo, "echo\0");
  if (!__test_string("Echo this string\n", 17, 1))
    __ms2_echo[1] = "FAIL - Echoed text does not match expected input text\0";

  // Echo test 3
  __ms_stdout_expects = 17;
  __ms_stdin_expects = 18;
  __clear_string();
  _prep_stdin("differenter text\n\n\0");
  result = (int)_ms_safe_call((void (*)(void))builtin_echo, "echo\0");
  if (_ms_recovery_status) {
    __ms_stdout_expects = 18;
    __ms_stdin_expects = 18;
    __clear_string();
    _prep_stdin("differenter text\n\n\0");
    (int)_ms_safe_call((void (*)(void))builtin_echo, "echo\0");
    if(_ms_recovery_status)
      __ms2_echo[2] = "FAIL - Function did not return on an empty line\0";
    else
      __ms2_echo[2] = "FAIL - Function prints final newline character (should return only)\0";
  }
  if (result != 16)
    __ms2_echo[4] = "FAIL - Function returned a different character count than expected\0";
  
  // Hello test 1
  __ms_stdout_expects = 21;
  __clear_string();
  result = (int)_ms_safe_call((void (*)(void))builtin_hello, "hello");
  if (_ms_recovery_status) {
    __ms2_general[2] = "FAIL - function did not return\0";
    __ms2_hello[0] = "FAIL - 'uart_putc' Called too many times\0";
  }
  else if (!__test_string("Error - bad argument\n", 21, 1))
    __ms2_hello[0] = "FAIL - Error message does not match expected text\0";
  if (result != 1)
    __ms2_hello[2] = "FAIL - Error code not returned when no argument given\0";

  // Hello test 2
  __ms_stdout_expects = 17;
  __clear_string();
  result = (int)_ms_safe_call((void (*)(void))builtin_hello, "hello username");
  
  if (_ms_recovery_status) {
    __ms2_general[2] = "FAIL - function did not return\0";
    __ms2_hello[1] = "FAIL - 'uart_putc' Called too many times\0";
  }
  else if (!__test_string("Hello, username!\n", 17, 1))
    __ms2_hello[1] = "FAIL - Message does not match expected text\0";
  if (result)
    __ms2_hello[3] = "FAIL - Non-0 value returned\0";

  // Shell test 1
  __ms_stdout_expects = 23;
  __ms_stdin_expects = 8;
  __clear_string();
  _prep_stdin("badcall\n\0");
  result = (int)_ms_safe_call((void (*)(void))shell, "\0");
  if (_ms_recovery_status == 2)
    __ms2_shell[1] = "FAIL - Reads from `uart_getc` after receiving '\n' character\0";
  else if (!__test_string("bareOS$ Unknown command\n\0", 24, 1))
    __ms2_shell[0] = "FAIL - Error message does not match expected text\0";


  // Shell test 2
  __ms_stdout_expects = 22;
  __ms_stdin_expects = 12;
  __clear_string();
  _prep_stdin("hello value\n\0");
  result = (int)_ms_safe_call((void (*)(void))shell, "\0");
  if (!__test_string("bareOS$ Hello, value!\n\0", 22, 1))
    __ms2_shell[3] = "FAIL - `hello` not called or printed value incorrect\0";

  // Shell test 3
  __ms_stdout_expects = 15;
  __ms_stdin_expects = 12;
  __clear_string();
  _prep_stdin("echo foobar\n\0");
  result = (int)_ms_safe_call((void (*)(void))shell, "\0");
  if (!__test_string("bareOS$ foobar\n\0", 15, 1))
    __ms2_shell[2] = "FAIL - `echo` not called or printed value incorrect\0";
  
  // Shell test 4
  __ms_stdout_expects = 39;
  __ms_stdin_expects = 14;
  __clear_string();
  _prep_stdin("hello\necho $?\n\0");
  result = (int)_ms_safe_call((void (*)(void))shell, "\0");
  if (_ms_recovery_status != 1)
    __ms2_shell[4] = "FAIL - `shell` did not loop\n\0";
  if (!__test_string("bareOS$ Error - bad argument\nbareOS$ 1\n\0", 39, 1))
    __ms2_shell[5] = "FAIL - Return substitution failed\0";

  _psudoprint("\nGeneral Tests:\0");
  for (int i=0; i<3; i++) {
    _psudoprint(__ms2_general_tests[i]);
    _psudoprint(__ms2_general[i]);
  }
  _psudoprint("\n\nEcho Tests:\0");
  for (int i=0; i<5; i++) {
    _psudoprint(__ms2_echo_tests[i]);
    _psudoprint(__ms2_echo[i]);
  }
  _psudoprint("\n\nHello Tests:\0");
  for (int i=0; i<4; i++) {
    _psudoprint(__ms2_hello_tests[i]);
    _psudoprint(__ms2_hello[i]);
  }
  _psudoprint("\n\nShell Tests:\0");
  for (int i=0; i<6; i++) {
    _psudoprint(__ms2_shell_tests[i]);
    _psudoprint(__ms2_shell[i]);
  }
  _psudoprint("\n\n");
}
