#include <barelib.h>

#define T_PUTC_BUFF 1024
#define T_GETC_BUFF 1024

static char putc_buffer[T_PUTC_BUFF];
static char getc_buffer[T_GETC_BUFF];
static uint32 in_idx = 0;
static uint32 out_idx = 0;
int __ms_stdout_search = '\0';
int __ms_stdout_expects = -1;
int __ms_stdin_expects = -1;
int __ms_syscall_expects = -1;

void _ms_recover(int);
char (*oldhook_out)(char);
char (*oldhook_in)(void);

void _prep_stdin(const char* str) {
  for (int i=0; str[i] != '\0' && i < T_GETC_BUFF; i++)
    getc_buffer[i] = str[i];
}

void syscall_tester(void) {
  if (__ms_syscall_expects >= 0)
    if (--__ms_syscall_expects < 0)
      _ms_recover(3);
}

char getc_tester(void) {
  if (getc_buffer[in_idx % T_PUTC_BUFF] == '\0')
    _ms_recover(2);
  return getc_buffer[in_idx++];
}

char putc_tester(char ch) {
  putc_buffer[out_idx++ % T_PUTC_BUFF] = ch;

  if (__ms_stdout_search == ch || __ms_stdout_search == '\0')
    if (__ms_stdout_expects >= 0)
      if (--__ms_stdout_expects < 0)
	_ms_recover(1);
  return ch;
}

void _psudoprint(const char* str) {
  while (*str != '\0') oldhook_out(*str++);
}

void __clear_string(void) {
  for (int i=0; i<T_PUTC_BUFF; i++)
    putc_buffer[i] = 0;
  for (int i=0; i<T_GETC_BUFF; i++)
    getc_buffer[i] = 0;
  out_idx = in_idx = 0;
}

char __test_template(const char* target) {
  int i = 0,j = 0;
  char test = 1;
  while (putc_buffer[i] != '\0') {
    if (target[j] == '%' && target[j+1] == 'd') {
      while (putc_buffer[i] >= '0' && putc_buffer[i] <= '9') i++;
      j += 2;
    }
    test &= putc_buffer[i++] == target[j++];
  }
  return test;
}

char __test_string(const char* target, int len, int clear) {
  int i = 0;
  char test = 1;
  for (; i<len; i++)
    test &= putc_buffer[i] == target[i];
  if (clear)
    __clear_string();
  return test;
}

char* _copy_string(char* str, const char* value) {
  while ((*(str++) = *(value++)) != '\0');
  return str - 1;
}

char* _copy_hex(char* str, unsigned int v) {
  int len = 0;
  int vals[20];
  for (int i=0; i<20; i++) vals[i] = 0;
  while (v) {
    vals[len++] = v % 16;
    v = v / 16;
  }
  int l = len - 1;
  for (; l >=0; l--) {
    *(str++) = vals[l] <= 9 ? vals[l] + '0' : 'a' + (vals[l] - 10);
  }
  return str;
}

char* _copy_dec(char* str, int v) {
  int len = 0;
  int vals[20];
  if (v < 0)
    *(str++) = '-';
  for (int i=0; i<20; i++) vals[i] = 0;
  while (v) {
    vals[len++] = v % 10;
    v = v / 10;
  }
  int l = len - 1;
  for (; l >=0; l--)
    *(str++) = vals[l] + '0';
  return str;
}
