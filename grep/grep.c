#include "grep.h"
#include "../alloc.h"

/*
 * Grep searches for a pattern within a file. 
 * 	To recreate this functionality:
 * 		1. Open file.
 * 		2. Pull a line and parse it for a pattern using strtok
 */
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

char *read_line(FILE *file_pointer) {
	// TODO: Rewrite
	
	size_t buff_size = BUFF_INCR;
	size_t pointer_pos = 0;
	size_t line_len = 0;
	char *line_pointer = (char *) error_checked_malloc(BUFF_INCR);
	char *buffer_pointer = (char *) error_checked_malloc(BUFF_INCR);
	
	for(;;) {
		line_pointer = fgets(line_pointer, buff_size, file_pointer);

		if (line_pointer == NULL) {
			break;
		}
		
		line_len = strlen(line_pointer);
		strncpy(buffer_pointer+pointer_pos, line_pointer, line_len);
		
		if ( line_pointer[line_len-1] == '\n') {
			pointer_pos += line_len;
			break;
		}
		
		if (pointer_pos < LINE_LIMIT) {
			// TODO: Fix bug with reallocation (see: pointer_pos).
			buffer_pointer = (char *) error_checked_realloc(buffer_pointer, buff_size+BUFF_INCR);
			pointer_pos = buff_size-1;
			buff_size += BUFF_INCR;
		} else 
		{
			fprintf(stderr, "Error whilst parsing line into buffer of size %u.\n", buff_size);
			exit(0);
		}
	}
	
	// I think fgets does this already, but just to be sure.
	buffer_pointer[pointer_pos-1] = '\0';
	free(line_pointer);
	fprintf(stdout, "---- \n%s\n", buffer_pointer);
	return buffer_pointer;
}

// TODO: Parse input arguments.
int main(int argc, char **argv) {
	struct args *a = parse_iargs(argc, argv, 2);
	FILE *file_ptr = fopen(a->filename, "r");
	if ( file_ptr == NULL ) {
		fprintf(stderr, "Error opening file %s\n", a->filename);
		exit(0);
	}
	
	char *line = read_line(file_ptr);
	fclose(file_ptr);
	free(line);
	free(a->filename);
	free(a->pattern);
	free(a);
}
