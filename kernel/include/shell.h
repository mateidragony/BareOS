#ifndef H_SHELL
#define H_SHELL

#define PROMPT "bareOS"  /*  Prompt printed by the shell to the user  */
#define PROMPT_SEP "$ "  // between prompt and user command

unsigned char shell(char*);
unsigned char builtin_echo(char*);
unsigned char builtin_hello(char*);
unsigned char builtin_clear();
unsigned char builtin_welcome();

// char *cmd_ll(char*,unsigned char);

typedef struct _shell_cmd {
    char *line;
    struct _shell_cmd *prev;
    struct _shell_cmd *next;
} shell_cmd_t;


#endif // shell.h