#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../alloc.h"

int regex_find(char*, char*);
int regex_match(char*, char*);
int multi_match_single_char(unsigned, char, char*, char*);
char *create_group(char*);
