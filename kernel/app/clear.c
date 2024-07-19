#include <bareio.h>
#include <barelib.h>


/*
 * 'builtin_clear'
 */
byte builtin_clear() {
  printf("\033[H");
  printf("\033[2J");
  printf("\n"); 
  return 0;
}
