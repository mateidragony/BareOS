#include <bareio.h>
#include <barelib.h>
#include <ctype.h>
#include <string.h>
#include <shell.h>
#include <thread.h>
#include <queue.h>
#include <malloc.h>
#include <trie.h>
#include <sleep.h>
#include <semaphore.h>
#include <fs.h>

/*
  TODO:
  - Add delete capability
*/

/*********************
 *  Builtin commands
**********************/

static bool color_on = !RUNNING_AG_TESTS;

byte set_color_mode([[maybe_unused]]char *line){
  color_on = !color_on;
  return EXIT_SUCCESS;
}

static const char *cmds[]      = {"clear", "cls", "echo", "hello", "welcome", "setcolormode", "alloclist", "semaphores", "fsd", "fsmask", "fsofd", "ls", NULL};
static const void *cmd_funcs[] = {&builtin_clear, &builtin_clear, &builtin_echo,
                                  &builtin_hello, &builtin_welcome, &set_color_mode,
                                  &public_print_alloc_list, &sem_main, &fs_print_fsd,
                                  &fs_print_mask, &fs_print_oft, &fs_print_root};

/*****************
 *  Command Trie
******************/

static trie_t *trie_head = NULL;

byte init_cmd_trie(){
  if(trie_init(&trie_head) == EXIT_FAILURE) return EXIT_FAILURE;
  for(int i=0; cmds[i] != NULL; ++i) if(trie_add(trie_head, cmds[i]) == EXIT_FAILURE) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}


byte t_call_func(byte (*func)(char*), char *args){
  uint32 tid = create_thread(func, args, strlen(args));
  resume_thread(tid);
  return join_thread(tid);
}

const void *get_command(const char* cmd_str){
  for(int i=0; cmds[i] != NULL; ++i) if(strcmp(cmds[i], cmd_str) == 0) return cmd_funcs[i];
  return NULL;
}


/*********************************
 *  Command history linked list  *
**********************************/

static shell_cmd_t *cur_cmd = NULL; // when user types, cur_cmd is the one being changed

byte init_cmd_ll(){
  if((cur_cmd = malloc(sizeof(shell_cmd_t))) == NULL) return EXIT_FAILURE;
  cur_cmd->line = NULL; // head will always have null line
  cur_cmd->prev = NULL;
  cur_cmd->next = NULL;
  return EXIT_SUCCESS;
}

char *cmd_ll_prev(){
  if(cur_cmd->prev == NULL) return NULL;
  cur_cmd = cur_cmd->prev;
  return cur_cmd->line;
}

char *cmd_ll_next(){
  if(cur_cmd->next == NULL) return "";
  cur_cmd = cur_cmd->next;
  return cur_cmd->line;
}

byte cmd_ll_add(char *line){
  while(cur_cmd->next != NULL) cur_cmd = cur_cmd->next; // go to end
  if((cur_cmd->next = malloc(sizeof(shell_cmd_t))) == NULL) return EXIT_FAILURE;
  cur_cmd->next->prev = cur_cmd;  // set prev
  cur_cmd = cur_cmd->next;        // move cur_cmd up
  cur_cmd->next = NULL;           // no next
  cur_cmd->line = line;           // set line ptr
  return EXIT_SUCCESS;
}


/*************************
 * Shell helper methonds
**************************/

byte parse_line(char *line, int line_len){
  byte return_code = 0;

  int space_idx = -1;
  for(int i=0; i<line_len; ++i){
    if(line[i] == ' '){ // str tokenize first space
      line[i] = '\0';
      space_idx = i;
      break;
    }
  }

  if(white_space_string(line)) return return_code;

  const void *cmd = get_command(line);
  if(space_idx != -1) line[space_idx] = ' '; // put space back

  if(cmd == NULL){
    if(color_on) printf("\033[38;2;252;55;58m"); // red

    if(RUNNING_AG_TESTS) printf("Unknown command\n");
    else printf("%s: command not found\n", line);
    return EXIT_FAILURE;
  } else{
    if(color_on) printf("\033[38;2;124;255;163m"); // green
    return t_call_func(cmd, line);
  }
}


void prtnum_buf(char *buf, int *buf_idx, byte num){

  if(num == 0){ // if the number is 0
    buf[(*buf_idx)++] = '0';
    return;
  } else if(num < 0){ // put in the negative sign
    buf[(*buf_idx)++] = '-';
    num*=-1;
  }

  int biggestBase = 1;
  while(num > biggestBase) biggestBase *= 10;
  biggestBase /= 10;

  if(biggestBase == 0){ // num is 1 or -1
    buf[(*buf_idx)++] = '1';
    return;
  }

  while(biggestBase > 0){
    buf[(*buf_idx)++] = (num / biggestBase) + '0';
    num -= (num / biggestBase) * biggestBase;
    biggestBase /= 10;
  }
}


byte line_replace_exit_code (char **line, int *line_len, int prev_ret){

  // count the number of replavcements to be made
  int num_replacements = 0;
  for(int i=0; i<*line_len; ++i){
    if((*line)[i] == '$' && i != *line_len-1 && (*line)[i+1] == '?'){ // something that should be replaced
      num_replacements++;
      ++i; // skip past '?'
    }
  }

  // figure out how many characters each replacement will take
  int num_chars = 1;
  int _div = 1;
  while(prev_ret / (_div *= 10) != 0) num_chars++;

  // once line has been entered, replace $ with exit code
  int updated_idx = 0;
  // allocate the exact space needed to replace all exit codes
  char *updated_line_buf = malloc(*line_len + (num_chars - 2) * num_replacements); // -2 because of $?


  for(int i=0; i<*line_len; ++i){
    if((*line)[i] == '$' && i != *line_len-1 && (*line)[i+1] == '?'){ // replace with last exit code
      prtnum_buf(updated_line_buf, &updated_idx, prev_ret);
      ++i; // skip past '?'
    } else {
      updated_line_buf[updated_idx++] = (*line)[i];
    }
  }

  *line_len = updated_idx;
  free(*line);
  *line = updated_line_buf;

  (*line)[*line_len] = '\0';

  return EXIT_SUCCESS;
}


/***********************************
 * Shell get line wrapper function *
 * that handles special behavior   *
 * for certain characters          *
************************************/

byte get_line_wrapper(char c, char **line_buf, int *line_len, int *line_buf_sz, int *cursor_idx, bool escape){
  static int tab_idx = -1;
  static char **ac_words = NULL;
  static int num_ac_words = -1;

  if(c == '\t'){ // tab autocomplete
    if(ac_words == NULL){ // not in process of autocompleting another word, so autocomplete this one
      trie_auto_complete(&ac_words, *line_buf, trie_head, &num_ac_words);
      tab_idx = 0;
      if(num_ac_words == 0) return EXIT_SUCCESS; // no words to print out
    } else tab_idx++;     // I am currently autocompleting, so move on to next index

    printf("\0338");                                     // move back to home
    printf("\033[K");                                    // clear everything

    char *ac_word = ac_words[tab_idx % num_ac_words];    // get ac_word
    int ac_word_len = strlen(ac_word);                   // get str length
    if(ac_word_len > *line_buf_sz){                      // realloc line_buf
      *line_buf_sz = ac_word_len;                           // increase buffer size
      char *temp = realloc(*line_buf, *line_buf_sz);        // realloc into temp
      if(temp == NULL) return EXIT_FAILURE;                 // check if realloc failed
      *line_buf = temp;                                     // set line_buf to realloc
    }                                                    // end realloc block
    memcpy(*line_buf, ac_word, ac_word_len);             // set line buf to ac word
    *line_len = ac_word_len;                             // set line_len to its new length

    (*line_buf)[*line_len] = '\0';                       // null terminate
    printf("%s", *line_buf);                             // reprint buffer
    *cursor_idx = *line_len;                             // move cursor_idx back

    return EXIT_SUCCESS;                                 // done handling tab
  }
  if(escape){ // some sort of escape code
    // if(c == 'A') printf(cmd_ll_prev());
    // else if(c == 'B') printf(cmd_ll_next());
    return EXIT_SUCCESS;
  }

  for(int i=0; i<num_ac_words; ++i) free(ac_words[i]);   // free ac_words
  if(num_ac_words != -1) free(ac_words);                 // free the actual ac_words

  tab_idx = -1;      //
  ac_words = NULL;   // Not tabbing anymore, so reset
  num_ac_words = -1; //

  return EXIT_SUCCESS;
}



/*******************************************************
 * 'shell' loops forever, prompting the user for input,
 * then calling a function based
 * on the text read in from the user.
 *******************************************************/

byte __ag_shell();

byte shell(char* arg) {

  if(RUNNING_AG_TESTS) return __ag_shell();



  byte prev_ret = 0;

  if(!RUNNING_AG_TESTS){
    if(init_cmd_trie() == EXIT_FAILURE) return EXIT_FAILURE;
    if(init_cmd_ll() == EXIT_FAILURE) return EXIT_FAILURE;
  }

  while(true){
    if(color_on) printf("\033[38;2;98;202;247m"); // turn color to blue

    printf(PROMPT);
    if(color_on) printf("\033[38;2;126;98;247m"); // turn color to pink

    printf(PROMPT_SEP);
    if(color_on) printf("\033[39m"); // reset color


    byte err_val;


    char *line;

    if((line = malloc(LINE_BUF_LEN)) == NULL) return EXIT_FAILURE;

    if(!RUNNING_AG_TESTS) cmd_ll_add(line);

    int line_len;
    int line_buf_sz = LINE_BUF_LEN;


    if(RUNNING_AG_TESTS) shitty_get_line_for_ag(line, &line_len, line_buf_sz); 
    else {
      err_val = get_line(&line, &line_len, &line_buf_sz, color_on, &get_line_wrapper);
      if(err_val == EXIT_FAILURE)printf("\nMaSh: Internal Error\n");
    }
    
    if(color_on) printf("\033[39m"); // reset color

    err_val = line_replace_exit_code(&line, &line_len, prev_ret);
    if(err_val == EXIT_FAILURE) printf("\nMash: Internal Error\n"); // Mash: Matei Shell (MaSh)

    prev_ret = parse_line(line, line_len);

    if(prev_ret == 98) return -1;

    if(color_on) printf("\033[39m"); // reset color

    cur_cmd->line = line;

    // free(line);
  }

  return prev_ret;
}


















void __ag_line_replace_exit_code (char line[LINE_BUF_LEN], int *line_len, int prev_ret){

  int updated_idx = 0;
  char updated_line_buf[LINE_BUF_LEN]; 

  for(int i=0; i<*line_len; ++i){
    if(line[i] == '$' && i != *line_len-1 && line[i+1] == '?'){ // replace with last exit code
      prtnum_buf(updated_line_buf, &updated_idx, prev_ret);
      ++i; // skip past '?'
    } else {
      updated_line_buf[updated_idx++] = line[i];
    }
  }

  *line_len = updated_idx;
  for(int i=0; i<updated_idx; ++i) line[i] = updated_line_buf[i];
  line[*line_len] = '\0';
}



byte __ag_shell(){

  byte prev_ret = 0;
  while(true){
    printf("bareOS$ ");

    char line[LINE_BUF_LEN];
    int line_len;
    int line_buf_sz = LINE_BUF_LEN;

    shitty_get_line_for_ag(line, &line_len, line_buf_sz); 

    __ag_line_replace_exit_code(line, &line_len, prev_ret);

    prev_ret = parse_line(line, line_len);

    if(prev_ret == 98) return -1;

  }

  return prev_ret;

}
