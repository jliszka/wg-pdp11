int trim(char * str) {
    int len = 0;

    // Find the end of the string
    for (; str[len] != 0; len++);

    // Zero out trailing whitespace
    for (; len >= 0; len--) {
        char c = str[len-1];
        if (c == ' ' || c == '\r' || c == '\n') {
            str[len-1] = 0;
        } else break;
    }
    return len;
}
