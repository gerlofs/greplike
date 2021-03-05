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
	struct opts *next;
};

struct file {
	char *filename;
	size_t length;
};

struct arguments {
	struct file *files;
	size_t num_files;
	char *expression;
	int flags; 
	/*	Flags:
	*	0: Return line numbers.
	*	1: Return filenames.
	*	2: Return match count.
	*	
	*/
};

struct arguments *append_file(struct arguments*, char*);
struct arguments *set_expression(struct arguments*, char*);
struct opts *append_option(struct opts*, char, int);
struct arguments *parse_arguments(int, char**);
int run_matching(struct arguments*);
void free_options(struct opts*);

#endif /* GREP_H */
