#include "grep.h"
#include "../alloc.h"
#include "../file.h"

struct args *parse_iargs(int argc, char **argv, size_t nargs) {
	if ( argc > (++nargs) ) {
		printf("Invalid number of parameters: %d, %u required\n", argc, nargs);
		exit(0);
	}
	
	struct args *a = (struct args *) error_checked_malloc(sizeof(struct args));	
	
	a->pattern = (char *) error_checked_malloc(strlen(argv[1]));
	strncpy(a->pattern, argv[1], strlen(argv[1]));
	a->pattern[strlen(argv[1])] = '\0';
	a->filename = (char *) error_checked_malloc(strlen(argv[2]));
	strncpy(a->filename, argv[2], strlen(argv[2]));
	a->filename[strlen(argv[2])] = '\0';

	return a;

}

// strtok() can be used to parse line into tokens and then check each token, but we're going
// to use regular expressions instead.
// TODO: Regex lines to find matching word.
// Possible options: build a primitive regex engine.
// 					use PCRE - this is what git's grep utility does.
// 
int main(int argc, char **argv) {
	struct args *a = parse_iargs(argc, argv, 2);
	FILE *file_ptr = fopen(a->filename, "r");
	if ( file_ptr == NULL ) {
		fprintf(stderr, "Error opening file %s\n", a->filename);
		exit(0);
	}
	
	char *line = read_line(file_ptr);
	line = read_line(file_ptr);
	line = read_line(file_ptr);
	line = read_line(file_ptr);
	line = read_line(file_ptr);
	line = read_line(file_ptr);
	line = read_line(file_ptr);
	fclose(file_ptr);
	free(line);
	free(a->filename);
	free(a->pattern);
	free(a);
}
