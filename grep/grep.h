#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define LINE_LIMIT 2048 // Standard UNIX line limit.

struct args {
	char *pattern;
	char *filename;
};

struct args *parse_iargs(int, char**);
char *read_line(FILE*);

