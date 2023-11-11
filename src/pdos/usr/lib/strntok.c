int strntok(char * str, char delim, char * tokens[], int ntokens)
{
    int token_count;
    for (token_count = 0; (token_count < ntokens) && (*str != 0); token_count++) {
        // Skip delimiter tokens to find the beginning of the token.
        while (*str == delim && *str != 0) str++;

        // No additional token, return what we have.
        if (*str == 0) return token_count;

        // Found the beginning of a token.
        tokens[token_count] = str;

        // Find the end of the token.
        while (*str != delim && *str != 0) str++;

        // End of the string! Return.
        if (*str == 0) return token_count+1;

        // Found the end of the token. Replace the delimiter with a \0.
        *str++ = 0;
    }
    return token_count;
}
