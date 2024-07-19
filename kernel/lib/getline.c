#include <bareio.h>
#include <barelib.h>
#include <ctype.h>
#include <shell.h>
#include <string.h>
#include <malloc.h>
#include <tty.h>

#define NUM_INPUT_COLORS 5;


void shift_buff_one(char *buf, int idx, int buf_sz){
  while(buf[idx] != '\0' && idx < buf_sz){
    buf[idx] = buf[idx+1];
    idx++;
  }
  buf[idx] = '\0'; // ensure null-termination
}



byte get_line(char **line_buf, int *line_len, int *line_buf_sz_ptr, bool color_on, byte(*wrapper)(char, char**, int*, int*, int*, bool)){

  printf("\0337"); // save cursor position

  *line_len = 0;
  char c;
  int line_buf_sz;

  if(*line_buf == NULL){
    line_buf_sz = LINE_BUF_LEN;
    if((*line_buf = malloc(LINE_BUF_LEN)) == NULL) return EXIT_FAILURE;
  } else line_buf_sz = *line_buf_sz_ptr;
  // memset(*line_buf, '\0', LINE_BUF_LEN);

  bool potential_escape_code = false;
  bool in_escape_code = false;
  int cursor_idx = 0;

  while((c = tty_getc()) != '\n'){ // while not new line

    bool wrapper_escape = false;

    (*line_buf)[*line_len] = '\0';

    // ----------
    // Escape codes (all escope codes start with ESC + [)
    // ----------
    if(c == 27){                                        // ^ potential escape code start
      potential_escape_code = true;
      continue;
    } else if(potential_escape_code && c == '['){       // successful escape code (officially in escape code)
      in_escape_code = true;
      potential_escape_code = false;
      continue;
    } else if(potential_escape_code){                   // failed escape code
      potential_escape_code = false;
      continue;
    }

    if(in_escape_code && c == ';'){                      // escape code intermediary
      continue;
    } else if(in_escape_code && is_digit(c)){            // digit stays in escape code (in general. some edge cases which idc about)
      continue;
    } else if(in_escape_code){                           // escape code end
      in_escape_code = false;
      potential_escape_code = false;
      wrapper_escape = true;

      if(c == 'A') printf("\033[B");                  // Up
      else if(c == 'B') printf("\033[A");             // Down
      else if(c == 'C'){                              // Right
        if(cursor_idx == *line_len) printf("\033[D");    // no more right
        else cursor_idx++;                               // can move right
      } else if(c == 'D'){                            // Left
        if(cursor_idx == 0) printf("\033[C");           // no more left
        else cursor_idx--;                              // can move left
      }
      goto wrapper;
    }

    if(c == '\t'){ // tab
      printf("\0338");                                                  // move back to home
      printf("\033[K");                                                 // clear everything
      printf("%s", *line_buf);                                          // reprint buffer
      cursor_idx = *line_len;                                           // move cursor_idx back

      goto wrapper;                                                     // wrapper function!
    }

    if(c == 127  && cursor_idx >= 1){   // backspace
      shift_buff_one(*line_buf, cursor_idx-1, *line_len);  // delete char from buf
      printf("\033[%dD", cursor_idx);                      // move back to home
      printf("\033[K");                                    // clear everything
      printf("%s", *line_buf);                             // reprint buffer
      cursor_idx--;                                        // move cursor back
      (*line_len)--;                                       // decrease line len
      if(*line_len) printf("\033[%dD", *line_len);         // move back to home
      if(cursor_idx) printf("\033[%dC",cursor_idx);        // move cursor to where it should be

      goto wrapper;                                        // wrapper function!
    }

    // ---------
    // Characters
    // ---------

    if(c >= 32 && c <= 126){ // printable ASCII character
      (*line_buf)[cursor_idx++] = c;
      if(cursor_idx >= *line_len) (*line_len)++;

      // realloc


      if((*line_len >= line_buf_sz)){
        line_buf_sz += LINE_BUF_LEN;
        char *temp = realloc(*line_buf, line_buf_sz);
        if(temp == NULL) return EXIT_FAILURE; // realloc failed
        *line_buf = temp;
      }

      goto wrapper;
    }

    // ---------
    // Wrapper
    // ---------
    wrapper:
    (*line_buf)[*line_len] = '\0';
    if(wrapper != NULL) wrapper(c, line_buf, line_len, &line_buf_sz, &cursor_idx, wrapper_escape);  // wrapper function!
  }

  (*line_buf)[*line_len] = '\0';

  wrapper('\n', line_buf, line_len, &line_buf_sz, &cursor_idx, false);    // call wrapper on new line

  if(line_buf_sz_ptr != NULL) *line_buf_sz_ptr = line_buf_sz;

  return EXIT_SUCCESS;
}




byte shitty_get_line_for_ag(char *line_buf, int *line_len, int line_buf_sz){
  *line_len = 0;
  char c;

  while((c = uart_getc()) != '\n' && *line_len < line_buf_sz){ // while not new line
    line_buf[(*line_len)++] = c;
  }

  line_buf[*line_len] = '\0';

  return EXIT_SUCCESS;
}