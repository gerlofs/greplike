#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


struct args {
	char *pattern;
	char *filename;
};

struct args *parse_iargs(int, char**, size_t);
