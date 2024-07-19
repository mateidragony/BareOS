char is_digit(char c){
    return c >= '0' && c <= '9';
}

char to_lower(char c){
    return c | 32;
}

char to_upper(char c){
    return c & (~32);
}

char is_white_space(char c){
    return (c>=9 && c<=13) || c == 32;
}