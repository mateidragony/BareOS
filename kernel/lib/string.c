#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <barelib.h>

int strcmp(const char *str1, const char *str2){
    int idx = 0;
    while(str1[idx] != '\0' && str2[idx] != '\0'){
        if(str1[idx] != str2[idx]) break;
        idx++;
    }
    return str1[idx] == str2[idx] ? 0 : str1[idx] > str2[idx] ? -1 : 1;
}

size_t strlen(const char *str){
    size_t len = 0;
    if(str[0] == '\0') return 0;
    while(str[len++] != '\0');
    return len-1;
}

void strncpy(char *dest, const char *src, size_t n){
    for(int i=0; i<n && src[i] != '\0'; ++i) dest[i] = src[i];
}

char *strmncpy(const char *src, size_t size){
    char *dest;
    if((dest = malloc((size+1) * sizeof(char))) == NULL) return NULL;
    size_t src_len = strlen(src);
    for(size_t i=0; i<size; ++i) dest[i] = i >= src_len ? '\0' : src[i];
    dest[size] = '\0';
    return dest;
}



char white_space_string(const char *str){
    int idx = 0;
    while(str[idx] != '\0') if(!is_white_space(str[idx++])) return 0;
    return 1;
}


unsigned int hash_str(const char *str){
    unsigned int hash = 0;
    for (int i = 0; i < strlen(str); i++) hash += (int)str[i] * (i % 2 == 0 ? STR_PRIME_1 : STR_PRIME_2);
    return hash;
}