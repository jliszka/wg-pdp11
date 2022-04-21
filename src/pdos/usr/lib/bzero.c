void bzero(unsigned char * buf, int n) {
    for (int i = 0; i < n; i++) {
        buf[i] = 0;
    }
}
