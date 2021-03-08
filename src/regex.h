#ifndef REGEX_H
#define REGEX_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alloc.h"

// Linked list structure containing groups within the base expression.
typedef struct node {
	char *expression;
	size_t length;
	short match_flags;
	char match_char;
	struct node *next;
} expression_list;

int regex_find(char*, char*);
char *regex_match(char*, char*);
char *multi_match_single_char(unsigned, char, char*, char*);
char *match_group(expression_list*, char*, char*);
expression_list *create_group(char*);
expression_list *create_node();
void append_node(expression_list*, expression_list*);
void group_teardown();
expression_list *create_class(char *);
char *generate_range(char *, size_t, char, char);
char *match_class(expression_list*, char*, char*);

#endif
