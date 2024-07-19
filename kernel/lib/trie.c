#include <trie.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <barelib.h>
#include <bareio.h>


#define TRIE_BUFF_SZ 16       // not expecting too many autocomplete attempts 16

byte trie_init(trie_t **head){
    if((*head = malloc(sizeof(trie_t))) == NULL) return EXIT_FAILURE;
    for(int i=0; i<ALPHABET_NUM; ++i) (*head)->children[i] = NULL;
    (*head)->is_terminal = false;
    return EXIT_SUCCESS;
}

byte trie_add(trie_t *head, const char *word){

    debug_printf(DEBUG_TRIE, "trie_add: %s (%d)\n", word, strlen(word));

    trie_t *cur = head; 
    for(int i=0; i<strlen(word); ++i){
        int char_idx = to_lower(word[i]) - 'a'; // go from ascii to number
        if(cur->children[char_idx] == NULL){   // trie node doesn't exist yet
            debug_printf(DEBUG_TRIE, "\t\ttrie node doesn't exist yet (%c).\n", to_lower(word[i]));
            trie_t *next;
            if((trie_init(&next)) == EXIT_FAILURE) return EXIT_FAILURE;
            cur->children[char_idx] = next;
        }
        cur = cur->children[char_idx];         // next
    }
    cur->is_terminal = true;
    return EXIT_SUCCESS;
}

byte _auto_complete(char ***words, trie_t *cur, char *str, int *num_words, int str_len, int *words_buf_sz){

    str[str_len] = '\0'; // for debugging.
    debug_printf(DEBUG_TRIE, "\t\t_auto_complete: cur_str(%s), str_len(%d), num_words(%d)\n", str, str_len, *num_words);    

    char next_chars[ALPHABET_NUM];
    int num_next = 0;
    for(int i=0; i<ALPHABET_NUM; ++i) if(cur->children[i] != NULL) next_chars[num_next++] = i; // find potential next chars
    debug_printf(DEBUG_TRIE, "\t\t\t_auto_complete: found %d potential next chars\n", num_next);

    if(num_next == 0 && !cur->is_terminal) return EXIT_FAILURE; // non-terminal terminal node
    else if(cur->is_terminal){ // actual terminal node
        if(((*words)[(*num_words)++] = strmncpy(str, str_len)) == NULL) return EXIT_FAILURE; // add word to words
        if(*num_words == *words_buf_sz){
            if((*words = realloc(*words, (*words_buf_sz += TRIE_BUFF_SZ) * sizeof(char*))) == NULL) return EXIT_FAILURE; // increase buffer size
            debug_printf(DEBUG_TRIE, "\t\t\t\t_auto_complete: realloc words\n");
        }

        debug_printf(DEBUG_TRIE, "\t\t\t_auto_complete: found a terminal node. num_words(%d) word just added to words: %s\n", *num_words, (*words)[*num_words - 1]);
    }

    if(num_next == 0){ // terminal terminal node
        debug_printf(DEBUG_TRIE, "\t\t\t_auto_complete: terminal terminal node\n");
        free(str);
        return EXIT_SUCCESS;
    } else { // multiple potential words
        debug_printf(DEBUG_TRIE, "\t\t\t_auto_complete: non-terminal\n");
        for(int i=0; i<num_next; ++i){
            char *str_cpy;
            if((str_cpy = strmncpy(str, str_len+1)) == NULL) return EXIT_FAILURE; // copy str + 1 for extra character
            str_cpy[str_len] = next_chars[i] + 'a';

            str_cpy[str_len+1] = '\0'; // for debugging
            debug_printf(DEBUG_TRIE, "\t\t\t_auto_complete: potential option (%c): str_cpy(%s)\n", next_chars[i] + 'a', str_cpy);

            if(_auto_complete(words, cur->children[(int)next_chars[i]], str_cpy, num_words, str_len+1, words_buf_sz) == EXIT_FAILURE) return EXIT_FAILURE;
        }
        free(str); // str isn't needed anymore because of strmncpy
    }

    return EXIT_SUCCESS;
}

byte trie_auto_complete(char ***words, const char *word, trie_t *head, int *num_words){

    debug_printf(DEBUG_TRIE, "trie_auto_complete: %s\n", word);

    if(head == NULL) return EXIT_FAILURE;  // trie not initialized

    trie_t *cur = head;
    for(int i=0; i<strlen(word); ++i){
        int char_idx = to_lower(word[i]) - 'a'; // go from ascii to number
        if(cur->children[char_idx] == NULL){    // word not in trie
            debug_printf(DEBUG_TRIE, "trie_auto_complete: word not in trie\n");
            *num_words = 0;
            return EXIT_FAILURE;
        }
        cur = cur->children[char_idx]; 
    }

    if((*words = malloc(TRIE_BUFF_SZ * sizeof(char*))) == NULL) return 1; // out of memory
    memset(*words, NULL, TRIE_BUFF_SZ);
    
    char *word_buf;
    if((word_buf = strmncpy(word, strlen(word))) == NULL) return EXIT_FAILURE;

    *num_words = 0;
    int words_buf_sz = TRIE_BUFF_SZ;

    _auto_complete(words, cur, word_buf, num_words, strlen(word), &words_buf_sz);

    debug_printf(DEBUG_TRIE, "trie_auto_complete: finished num_words: %d\n", *num_words);
    return EXIT_SUCCESS;
}