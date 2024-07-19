#include <bareio.h>
#include <barelib.h>
#include <tty.h>

/*
 *  In this file, we will write a 'printf' function used for the rest of
 *  the semester.  In is recommended  you  try to make your  function as
 *  flexible as possible to allow you to implement  new features  in the
 *  future.
 *  Use "va_arg(ap, int)" to get the next argument in a variable length
 *  argument list.
 */

// I didn't want to do a bunch of if/switch statements lul

void printStr(char *str, int padding){
  int idx=0;
  int num_printed = 0;
  while(str != NULL && str[idx] != '\0'){
    tty_putc(str[idx++]);
    num_printed++;
  }
  for(int i=0; i<padding-num_printed; ++i) tty_putc(' ');
}

void printNum(uint64 num, int base, int padding){
  // largest 64 int: -9,223,372,036,854,775,808 (20 digits)
	
	if(num == 0){
    for(int i=0; i<padding-1; ++i) tty_putc('0');
    tty_putc('0');
    return;
  }

  int num_to_print = 0;
  uint64 biggestBase = 1;
  while(num >= biggestBase){
    num_to_print++;
    biggestBase *= base;
  }
  biggestBase /= base; 

  if(biggestBase == 0){
    for(int i=0; i<padding-1; ++i) tty_putc('0');
    tty_putc('1');
    return;
  }

  for(int i=0; i<padding-num_to_print; ++i) tty_putc('0');

  int chars = 0;
  while(biggestBase > 0){
    chars++;
    tty_putc((num / biggestBase) + '0' + ((num / biggestBase) >= 10 ? 'a' - 10 - '0' : 0));
    num -= (num / biggestBase) * biggestBase;
    biggestBase /= base;
  }
}

void printDec(int num, int padding){
	if(num < 0) {
		num *= -1;
		tty_putc('-');
	}
  printNum(num, 10, padding);
}

void printHex(uint64 num, int padding){
	tty_putc('0');
	tty_putc('x');
  printNum(num, 16, padding);
}

void _printf(const char *format, va_list ap){
  for(int i=0; format[i] != NULL; ++i){

    char c = format[i];

    if(c == '%'){ // potential formatter

      int padding = 0;
      int padding_base = 1;
      while(format[i+1] >= '0' && format[i+1] <= '9'){
        padding *= padding_base;
        padding_base *= 10;
        padding += format[i+1] - '0';
        i++;
      }

      if(format[i+1] == 'd'){ // decimal
				printDec(va_arg(ap, int), padding);
				i++;
				continue;
			} else if(format[i+1] == 'x'){ // hex
				printHex(va_arg(ap, uint64), padding);
				i++;
				continue;
			} else if(format[i+1] == 'c'){ // char
        tty_putc(va_arg(ap, int));
        i++;
        continue;
      } else if(format[i+1] == 's'){ // string
        printStr(va_arg(ap, char*), padding);
        i++;
        continue;
      }
    }

    tty_putc(c);
  }

  
  va_end(ap);
}

void printf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  _printf(format, ap);
}


#define DEBUG_MASK DEBUG_FILE

void debug_printf(int debug_ctx, const char *format, ...){
  va_list ap;
  va_start(ap, format);
  if(DEBUG_MASK & debug_ctx){
    switch (debug_ctx) {
      case DEBUG_MALLOC: printf("\033[93m"); break; // yellow
      case DEBUG_TRIE  : printf("\033[94m"); break; // orange
      case DEBUG_SLEEP : printf("\033[95m"); break; // purple
      case DEBUG_FILE  : printf("\033[96m"); break; // cyan
      default          : printf("\033[39m"); break; // reset  
    }

    printf("DEBUG: ");
    _printf(format, ap);
    printf("\033[39m"); // reset color
  }
}