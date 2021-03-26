/* GREPLIKE */
/* FILE.C */

/* Abstractions of fopen and fgets to open and read
 * files into a dynamically allocated buffer. 
*/

#include "file.h"

char *read_line(FILE *file_pointer) {
	/*
	 * Read line from file into a buffer of a starting, fixed size (BUFF_INCR).
	 * Copy this fixed buffer across to a dynamically-allocated buffer at an moving
	 * 	offset.
	 * Once we're at a line end (new line present), copy the remainder of the line
	 * 	from the fixed buffer to the dynamically-allocated buffer and free
	 * 	the line pointer. 
	 * The dynamic buffer is returned after being null-terminated.
	 *
	 * TODO: This should probably use fread or mmap with tracking offsets rather than 
	 * 	fgets. 
	 */

	size_t buff_size = BUFF_INCR;
	size_t pointer_pos = 0;
	char *line_pointer = (char *) error_checked_malloc(BUFF_INCR); // Static buffer.
	char *line_buffer = (char *) error_checked_malloc(BUFF_INCR); // Resizable buffer.
	char *rp = NULL;
	for(;;) {
		// Reads string buffer of BUFF_INCR bytes from fp into lp.
		// NULL signifies an error. 
		if (fgets(line_pointer, BUFF_INCR, file_pointer) == NULL) return NULL; // EOF.
		// Search for the new line.
		for ( rp = line_pointer+(BUFF_INCR-1); *rp != '\n' && rp > line_pointer; rp--);
		// If the new line exists, copy across the remainder of the line and break.
		if ( rp != line_pointer ) {
			strncpy(line_buffer+pointer_pos, line_pointer, rp-line_pointer);
			break;
		}	// End of line.
				
		// Increase the size of the buffer and copy contents.
		if (pointer_pos < LINE_LIMIT) {
			// Copy the static buffer into the dynamic buffer.
			line_buffer = (char *) error_checked_realloc(line_buffer, (buff_size+=BUFF_INCR)); // Increase buffer size by BUFF_INCR (128).
			strncpy(line_buffer+pointer_pos, line_pointer, BUFF_INCR-1); // Copy contents from line into buffer.
			pointer_pos += BUFF_INCR-1; // Move write pointer position to be at the start of the 'next' buffer segment.
		} else 
		{
			fprintf(stderr, "Error whilst parsing line into buffer of size %lu.\n", buff_size);
			exit(1);
		}
	}
	// Null-terminate the dynamic buffer.
	line_buffer[pointer_pos+(rp-line_pointer)] = '\0';
	free(line_pointer); // Free the static buffer.
	return line_buffer;
}

FILE *open_file(char *filename, size_t length) {
	/*
	 * Open a file, pretty self-explanatory.
	 * access() is used to check whether a) a file exists and b) whether it is readable.
	 */
	
	printf("Length: %lu, filename %s\n", length, filename);

	if ( length == 0 ) {
		fprintf(stderr, "Empty filename given\n");
		exit(1);
	} else if ( length == 1 && (*filename == 0x00 || *filename == 0x2F) ) {
		fprintf(stderr, "Invalid filename\n");
		exit(1);
	} else if ( access(filename, F_OK) == -1 ) {
	       	fprintf(stderr, "No file with the filename %s\n", filename);
		exit(errno);
	} else if ( access(filename, R_OK) == -1) {
		fprintf(stderr, "Filename %s cannot be read due to access restrictions\n", filename);
		exit(errno);
	}
	FILE *fp = fopen(filename, "r");
	if ( fp == NULL ) {
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	} else return fp;
}
