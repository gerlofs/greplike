#include <stdlib.h>
#include <stdio.h>
#include "../src/file.h"
#include "../src/regex.h"

static const char filename[] = "tests/single_line_regex.txt";

int main(void) {
	/*	Open single_line_regex file, parse each new line as a regular expression input or a input string.
	*		empty lines (\n only) seperate test inputs.
	*/
	
	int tests_run = 0;
	
	FILE *fp = fopen(filename, "r");
	char *line = NULL;
	if ( fp == NULL ) {
		fprintf(stdout, "%s\n", filename);
		exit(0);
	}
	
	do { 
		char *expression = read_line(fp); 
		char *input = read_line(fp);
		fprintf(stdout, "%s >> %s \n", expression, input);
		if ( regex_find(expression, input) == 0 ) {
			fprintf(stdout, "Test failed: %d !!!\n", tests_run);
			exit(0);
		} else tests_run++;
	} while ( (line=read_line(fp)) != NULL );
}