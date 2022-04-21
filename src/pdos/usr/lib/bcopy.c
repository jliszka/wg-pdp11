void bcopy(unsigned char * dst, unsigned char * src, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}
