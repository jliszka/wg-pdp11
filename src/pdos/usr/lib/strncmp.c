#include "stdlib.h"

int strncmp(char * a, char * b, int n)
{
    // Compare up to the first n characters
    for (int i = 0; i < n; i++)
    {
        char diff = b[i] - a[i];
        if (diff != 0) {
            return diff;
        }
        if (a[i] == 0) {
            break;
        }
    }
    return 0;
}
