
char getopt(int * argc, char *** argv, char * optstring, char ** optarg) {
	*optarg = 0;
	if (*argc == 1) {
		return -1;
	}
	if ((*argv)[1][0] != '-') {
		return -1;
	}
	char opt = (*argv)[1][1];
	*argv++;
	*argc--;

	char *s = optstring;
	while (*s != 0 && *s != opt) {
		s++;
	}
	if (s == 0) {
		return '?';
	}

	if (s[1] == ':') {
		*optarg = **argv;
		*argv++;
		*argc--;
	}

	return opt;
}
