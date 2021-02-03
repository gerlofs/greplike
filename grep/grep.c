#include "grep.h"
#include "../alloc.h"

/*
 * Grep searches for a pattern within a file. 
 * 	To recreate this functionality:
 * 		1. Open file.
 * 		2. Pull a line and parse it for a pattern using strtok
 */

struct args *parse_iargs(int argc, char **argv) {
	if ( argc > 3 ) {
		printf("Invalid number of parameters: %d, 3 required\n", argc);
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

char *read_line(FILE *file_pointer) {
	size_t buff_size = 128;
	size_t pointer_pos = 0;
	size_t line_len = 0;
	char *line_pointer = (char *) error_checked_malloc(line_size);
	char *write_pointer = p;
	for(;;) {
		
		if ((fgets(line_pointer, buff_size, file_pointer)) == NULL ) {
			printf("DONE\n");
			break;
		}
		
		line_len = strlen(lp);
		
		if ( lp[line_len-1] == '\n') {
			printf("DONE\n");
			break;
		}
		
		if (pointer_pos < LINE_LIMIT) {
			lp = (char *) error_checked_realloc(lp, );
			lp = lp + (line_size - line_size);
		} else 
		{
			printf("%s, %u, %u\n", lp, line_size, line_len);
			fprintf(stderr, "Error whilst parsing line into buffer of size %u.\n", line_size);
			exit(0);
		}
	}
	
	// I think fgets does this already, but just to be sure.
	lp[line_size-1] = '\0';
	return lp;
}

// TODO: Parse input arguments.
int main(int argc, char **argv) {
	struct args *a = parse_iargs(argc, argv);
	FILE *file_ptr = fopen(a->filename, "r");
	if ( file_ptr == NULL ) {
		fprintf(stderr, "Error opening file %s\n", a->filename);
		exit(0);
	}
	
	char *line = read_line(file_ptr);
	fprintf(stdout, "%s\n", line);
	fclose(file_ptr);
	free(line);
	free(a->filename);
	free(a->pattern);
	free(a);
}
