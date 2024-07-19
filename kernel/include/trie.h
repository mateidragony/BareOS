#ifndef TRIE_H
#define TRIE_H

#define ALPHABET_NUM 26

typedef struct _trie{
    struct _trie *children[ALPHABET_NUM];
    unsigned char is_terminal;
} trie_t;

unsigned char trie_init(trie_t**);
unsigned char trie_add(trie_t*, const char*);
unsigned char trie_auto_complete(char***, const char*, trie_t*, int*);

#endif // trie.h