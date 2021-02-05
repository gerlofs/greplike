#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define LINE_LIMIT 2048 // Standard UNIX line limit.
#define BUFF_INCR 128 	// Number of bytes to increase the buffer by each time we don't read a line end.

struct args {
	char *pattern;
	char *filename;
};

struct args *parse_iargs(int, char**, size_t);
