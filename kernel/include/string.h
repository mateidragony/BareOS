#ifndef H_STRING
#define H_STRING

#define STR_PRIME_1 53
#define STR_PRIME_2 97

int strcmp(const char *, const char *);
unsigned long strlen(const char *);
char white_space_string(const char*);
void strncpy(char*, const char*, unsigned long);
char *strmncpy(const char*, unsigned long);
unsigned int hash_str(const char*);

#endif // string.h