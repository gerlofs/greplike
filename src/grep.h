#ifndef GREP_H
#define GREP_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

struct opts {
	char optc;
	int optn; 
};

struct file {
	char *filename;
	size_t length;
};

struct arguments {
	struct file *files;
	size_t num_files;
	char *expression;
	int return_line_n;
};

struct arguments *append_file(struct arguments*, char*);

#endif /* GREP_H */