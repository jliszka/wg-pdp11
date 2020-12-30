
#define PTR_STATUS 0177550 // paper tape reader status word
#define PTR_DATA 0177552   // paper tape reader data buffer
#define PTR_ENABLE 1
#define PTR_READY (1 << 7)
#define PTR_ERROR (1 << 15)

// Paper tape reader interface
int ptr_status = 0;
unsigned char ptr_char;
int ptr_has_next()
{
    if (ptr_status == 1) {
        return 1;
    }
    volatile unsigned int *xcsr = (unsigned int *)PTR_STATUS;
    volatile unsigned char *xbuf = (unsigned char *)PTR_DATA;
    *xcsr |= PTR_ENABLE;
    while ((*xcsr & PTR_READY) == 0)
    {
        if (*xcsr & PTR_ERROR)
        {
            ptr_status = 0;
            return 0;
        }
    }
    ptr_char = *xbuf;
    ptr_status = 1;
    return 1;
}

unsigned char ptr_next()
{
    ptr_status = 0;
    return ptr_char;
}

int ptr_read(int bytes, unsigned char *dst)
{
    volatile unsigned int *xcsr = (unsigned int *)PTR_STATUS;
    volatile unsigned char *xbuf = (unsigned char *)PTR_DATA;
    int byte_count;
    for (byte_count = 0; byte_count < bytes; byte_count++)
    {
        *xcsr |= PTR_ENABLE;
        while ((*xcsr & PTR_READY) == 0)
        {
            if (*xcsr & PTR_ERROR)
            {
                return byte_count;
            }
        }
        *dst++ = *xbuf;
    }
    return byte_count;
}
