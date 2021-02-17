#ifndef REGEX
#define REGEX

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../alloc.h"

// Linked list structure containing groups within the base expression.
typedef struct g {
	char *regex; 
	size_t regex_len;
	int need_to_match;
	struct g *next;
} groups;

int regex_find(char*, char*);
int regex_match(char*, char*);
int multi_match_single_char(unsigned, char, char*, char*);
int multi_match_group(groups*, char*, char*);
groups *create_group(char*);
groups *create_node();
void print_list();
void append_node(groups*, groups*);
void group_teardown();

#endif