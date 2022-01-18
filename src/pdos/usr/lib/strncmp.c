#include "stdlib.h"

int strncmp(char * a, char * b, int n)
{
    // Compare the first n characters and the \0
    while (*a && *b && n--)
    {
        char diff = *b++ - *a++;
        if (diff != 0)
        {
            return diff;
        }
    }
    return *b - *a;
}
