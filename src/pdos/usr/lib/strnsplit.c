int strnsplit(char * str, char * delim, int len, char * tokens[], int ntokens) {
    int token_count;
    for (token_count = 0; token_count < ntokens; token_count++) {
        // Found the beginning of a token.
        tokens[token_count++] = str;

        // Find the delimiter
        while (strncmp(str, delim, n) != 0 && *str != 0) str++;

        // No delimiter found before end of the string
        if (*str == 0) return token_count;

        // Terminate the token and skip past the delimiter
        *str = 0;
        str += n;
    }
    return token_count;
}
