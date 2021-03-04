#include "grep.h"
#include "alloc.h"
#include "file.h"

// TODO:
// 1. Implement option parsing [DONE].
// 2. Implement -n (print line number of each line with regex_match).
// 3. Implement -i (print filename of files with lines returning positive matches).
// 4. Implement -c (count matches and return this value).

struct arguments *append_file(struct arguments *args, char *filename) {
	size_t filename_len = strlen(filename);
	struct file f;
	f.filename = (char *) error_checked_malloc(filename_len);
	f.length = filename_len;
	strncpy(f.filename, filename, filename_len);
	f.filename[filename_len] = 0x00;
	if ( args->files == NULL ) args->files = ( struct file * ) error_checked_malloc(sizeof(*args->files));
	else args->files = (struct file *) error_checked_realloc(args->files, sizeof(*args->files) * args->num_files);
	args->files[args->num_files] = f;
	args->num_files += 1;
	return args;
}

struct arguments *set_expression(struct arguments *args, char *expression) {
	size_t e_len = strlen(expression);
	args->expression = (char *) error_checked_malloc(e_len);
	strncpy(args->expression, expression, e_len);
	args->expression[e_len] = 0x00;	
	return args;
}

struct opts *append_option(struct opts *head, char optc, int optn) {
	struct opts *node = (struct opts *) error_checked_malloc(sizeof(struct opts));
	node->optc = optc;
	node->optn = optn;
	node->next = NULL;
	printf("Node: %d, %c\n", node->optn, node->optc);
	
	struct opts *seeker = head;
	if ( seeker == NULL ) return node;
	while ( seeker->next != NULL ) seeker = seeker->next;
	seeker->next = node;
	return head;
}

void free_options(struct opts *head) {
	struct opts *node;
	while ( head != NULL ) {
		node = head;
		head = head->next;
		free(node);
	}
}

struct arguments *parse_arguments(int argc, char **argv) {
	struct arguments *args = (struct arguments *) error_checked_malloc(sizeof(struct arguments));
	args->files = NULL; 
	args->num_files = 0;
	args->expression = NULL;
	args->flags = 0;

	struct opts *options = NULL;
	int nopts = 0;
	char *arg_ptr;
	int n;

	for ( n = 1; n < argc; n++ ) {
		arg_ptr = argv[n];
		if ( arg_ptr != NULL ) {
			if (*(arg_ptr++) == 0x2D ) {
				if ( *arg_ptr == 0x2D ) arg_ptr++;
				options = append_option(options, tolower(*arg_ptr), n);
				nopts++;
			}
		}
	}		

	struct opts *node = options;

	// Add clause where opti == 0, parse arg1 into filename and arg2 into expression.
	// If args->files == NULL, if args->expression == NULL.
	
	int end_index = 0;
	while(node != NULL) {
		printf("%d, %d\n", node->optn, node->next == NULL);
		if ( node->next == NULL ) end_index = argc;
		else end_index = node->next->optn;
		char c = node->optc;
		int i = node->optn;
		do {
			printf("-- %d, %c", i, c);
			switch ( (int) c ) {
				case 0x66: // -f
					args = append_file(args, argv[i]);
					break;
				case 0x65: // -e
					args = set_expression(args, argv[i]);
					break;
				case 0x6E: // -n 
					args->flags |= 0x01; // Set bit in flags for returning line numbers.
					break;
				case 0x69: // -i
					args->flags |= 0x02; // Set bit in flags for returning filenames.
					break;
				case 0x63: // -c 
					args->flags |= 0x04; // Set bit in flags for counting occurances.
					break;		
			}
		} while ( ++i < end_index );
	
		node = node->next;
	}

	if ( argc < 3 ) {
		fprintf(stdout, "No enough arguments, try again: greplike <expr> <filename>\n");
		exit(0);
	}
	
	if ( args->files == NULL ) {
		printf("Setting filename: %s\n", argv[2]);
		args = append_file(args, argv[2]);
	}
	if ( args->expression == NULL ) {
		printf("Setting expr: %s\n", argv[1]);
		args = set_expression(args, argv[1]);
	}
	
	free_options(options);
	return args;
}

int run_matching(struct arguments *args) {
	// Run regex match on lines from files using expression.
	// Collect information and return is based upon the bits set in args->flags.
	return 1;
}

int main(int argc, char **argv) {
	// Parse input arguments scanning for '-'.
	// greplike <expr> <filename> 
	// greplike -f <filename1>, <filename2>, <filename3> 
	
	struct arguments *args = parse_arguments(argc, argv);
	printf("%d, %s, %s, %d\n", args->num_files, args->files[0].filename, args->expression, args->flags);
	free(args);
}
