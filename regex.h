#ifndef REGEX
#define REGEX

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../alloc.h"

// Linked list structure containing groups within the base expression.
typedef struct node {
	char *expression;
	size_t length;
	int match_required;
	struct node *next;
} expression_list;

int regex_find(char*, char*);
int regex_match(char*, char*);
int multi_match_single_char(unsigned, char, char*, char*);
int match_group(expression_list*, char*, char*);
expression_list *create_group(char*);
expression_list *create_node();
void append_node(expression_list*, expression_list*);
void group_teardown();
expression_list *create_class(char *);
char *generate_range(char *, size_t, char, char);
int match_class(expression_list*, char*, char*);

#endif
