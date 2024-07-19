#include <bareio.h>
#include <barelib.h>
#include <string.h>

/*
 * 'builtin_hello' prints "Hello, <text>!\n" where <text> is the contents 
 * following "builtin_hello " in the argument and returns 0.  
 * If no text exists, print and error and return 1 instead.
 */
byte builtin_hello(char* arg) {

  arg += 6;

  if(strcmp(arg, "\0") == 0){
    printf("Error - bad argument\n");
    return 1;
  }

  printf("Hello, %s!\n", arg);
  return 0; 
}
