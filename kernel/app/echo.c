#include <bareio.h>
#include <barelib.h>
#include <string.h>
#include <shell.h>

/*
 * 'builtin_echo' reads in a line of text from the UART
 * and prints it.  It will continue to do this until the
 * line read from the UART is empty (indicated with a \n
 * followed immediately by another \n).
 */
byte builtin_echo(char* arg) {
  arg+=5; // don't print the command

  if(strlen(arg) > 0){ // regular echo
    printf("%s\n", arg);
    return 0;
  } 
 
  // loop echo
  char line[LINE_BUF_LEN];
  memset(line, '\0', LINE_BUF_LEN);

  int total_chars = 0;
  int line_chars = 0;
  while(1){
    char c = uart_getc();
    if(c == '\n' && line_chars == 0) return total_chars; // two new lines in a row
    else if(c == '\n'){                                  // one new line
      line[line_chars] = '\0';                              // null terminate line buffer
      printf("%s\n", line);                                 // print the line buffer
      line_chars = 0;                                       // reset num line chars
    } else {                                             // no new line
      line[line_chars++] = c;                               // update line buffer
      total_chars++;                                        // increase total chars typed

      if(line_chars == LINE_BUF_LEN){                       // buffer overflow
        printf("\nMash: Line Buffer Overflow\n");
        return 0;
      }
    }
  }
}
