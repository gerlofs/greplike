#include "grep.h"
#include "alloc.h"
#include "file.h"
#include "regex.h"

struct arguments *append_file(struct arguments *args, char *filename) {
	/*	Assign and allocate filename string to args->files string array.
	*	Increment the args->num-files counter.
	*	Return a new args struct containing the new file entry.
	*	
	*	TODO: Just make this return a string pointer or a pointer to a 
	*			linked list of filenames (this is preferable as it lets
	*			us add additional entries later, e.g. user ids and perms).
	*/
	
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
	/*	Assign and allocate the regular expression string to 
	*		args->expression.
	*	Null-terminate args->expression and return the args struct
	*		containing the new expression string.
	*/
	size_t e_len = strlen(expression);
	args->expression = (char *) error_checked_malloc(e_len+1);
	strncpy(args->expression, expression, e_len);
	args->expression[e_len] = (char) 0x00;	
	return args;
}

struct opts *append_option(struct opts *head, char optc, int optn) {
	/*	Assign and allocate a new node in the options linked list.
	*	Set the optc (character) and optn (argument index) of the 
	*		option node.
	*	Find the new position of the node by seeking through the 
	*		linked list until we reach the last node without a peer
	*		(node->next == NULL), set node->next to be the new node.
	*/
	
	struct opts *node = (struct opts *) error_checked_malloc(sizeof(struct opts));
	node->optc = optc;
	node->optn = optn;
	node->next = NULL;
	
	struct opts *seeker = head;
	if ( seeker == NULL ) return node;
	while ( seeker->next != NULL ) seeker = seeker->next;
	seeker->next = node;
	return head;
}

void free_options(struct opts *head) {
	/*	A simple freeing solution that uses a seek and temp pointer to 
	*		find the next contiguous allocated node in the list, and 
	*		assigns this node to the temp pointer, moves the seek pointer
	*		to the next node and then frees the previous (temp) node.
	*
	* e.g. node->node->node->NULL
	*		t		h
	*		f		t		h
	*		f		f				h
	*		f		f		f		h
	*/
	
	struct opts *node;
	while ( head != NULL ) {
		node = head;
		head = head->next;
		free(node);
	}
}

struct arguments *parse_arguments(int argc, char **argv) {
	/*	Parse command-line arguments which may be flags (-f) or options
	*		(--option), our tool will treat both identically as options
	*		and flags will have identical expected first characters, e.g.
	*		-f and --files. 
	*	First, allocate space for an 'args' struct containing a string array
	*		(list of files), an unsigned file count, a regular expression string,
	*		and an integer in which to store the flags which will dictate the 
	*		type of matching and information returned by the grep operation.
	*	Two parses are used to extract the command-line arguments, firstly a 
	*		once-through parse of the options is performed, extracting the optc
	*		(unique character), optn (index of the option/flag in the arguments 
	*		array), and the number of options (nopts). Secondly, for each node
	*		in the options linked list, parse the argv strings between the optn
	*		indeces and action the associated function for that option, e.g.
	*		-f adds a file for each filename string in the arguments list 
	*		between otpn = options[n]+1 and optn = options[n+1]-1.
	* 	If we don't have enough arguments ( < 2 plus the static argument at [0])
	*		or at least one filename and the expression is not assigned, this is
	*		done manually using argv[2] and argv[1] respectively.
	*	Examples:
	*	greplike <expr> <filename> [Base case]
	*	greplike -f <filename1> <filename2> -e <expression> -n [Grep f1 and f2 with
	*		e and print out the line numbers of each match.
	*
	*/
	
	if ( argc < 3 ) {
		fprintf(stdout, "No enough arguments, try again: greplike <expr> <filename>\n");
		exit(1);
	}
	
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
		if ( node->next == NULL ) end_index = argc;
		else end_index = node->next->optn;
		char c = node->optc;
		int i = node->optn+1;
		int invalid_opt = 0;
		if ( i == end_index ) {
			switch ( (int) c ) {	
				case 0x6E: // -n 
					args->flags |= 0x01; // Set bit in flags for returning line numbers.
					break;
				case 0x69: // -i
					args->flags |= 0x02; // Set bit in flags for returning filenames.
					break;
				case 0x63: // -c 
					args->flags |= 0x04; // Set bit in flags for counting occurances.
					break;
				default:
					invalid_opt++;

			}	
		}

		for (;i < end_index;i++) {
			printf("DEBUG: %c\n", c);
			switch ( (int) c ) {
				case 0x66: // -f
					args = append_file(args, argv[i]);
					break;
				case 0x65: // -e
					args = set_expression(args, argv[i]);
					break;	
				default:
					invalid_opt++;
			}
		}

		if ( invalid_opt ) {
			fprintf(stderr, "Invalid option %c, check README.md for information on valid options\n", c);
			exit(1);
		}

		invalid_opt = 0;
		node = node->next;
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
	/*	The meat'n'potatoes, initiates the pattern matching in a loop
	*		until the file list has been exhausted.
	*	A filename is read in from args->files and an open is attempted.
	*		If it succeeds, a line buffer is read from the file (which 
	*		is of variable length, capped at 2048 bytes) and used as the
	*		text input of regex_find, the result of which is tallied 
	*		for printing with the -c flag. 
	*	Flags are checked prior to matching to avoid doing bit shifts every
	*		line.
	*
	* TODO: Push file matches to an array of indeces for printing at the end
	*		rather than during a match.
	* TODO: Keep track of whether we're matching or not, return this to be returned
	* 	by main.
	*/
	
	unsigned match_count = 0;
	int line_print = is_bit_set(args->flags, 0);
	int file_print = is_bit_set(args->flags, 1);
	int count_print = is_bit_set(args->flags, 2);
	
	for ( int f = 0; f < args->num_files; f++ ) {
		unsigned line_number = 0;
		FILE *fp = open_file(args->files[f].filename);
		char *lb = NULL;
	    
	while ((lb=read_line(fp)) != NULL ) {
			line_number++;
			int match = regex_find(args->expression, lb);
			if ( match ) {
				match_count++;
				if ( line_print ) printf("Matched line: %u\n\n", line_number);
				else if ( file_print ) {
					printf("\nMatched in: %s\n", args->files[f].filename);
				}
			}
		}
	}

	if ( count_print ) printf("\nMatches: %u\n", match_count);

	return 1;
}

int main(int argc, char **argv) {	
	struct arguments *args = parse_arguments(argc, argv);
	run_matching(args);
	free(args);
}
