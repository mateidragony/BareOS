#include <bareio.h>
#include <barelib.h>

/*
 *  In this file, we will write a 'printf' function used for the rest of
 *  the semester.  In is recommended  you  try to make your  function as
 *  flexible as possible to allow you to implement  new features  in the
 *  future.
 *  Use "va_arg(ap, int)" to get the next argument in a variable length
 *  argument list.
 */

void printf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  
  
  
  va_end(ap);
}
