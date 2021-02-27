#ifndef FILE_H
#define FILE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "alloc.h"
#define LINE_LIMIT 2048 // Standard UNIX line limit.
#define BUFF_INCR 128 	// Number of bytes to increase the buffer by each time we don't read a line end.

char *read_line(FILE*);

char *read_line(FILE *file_pointer) {
	// TODO: Fix bug where last line of the file causes fault.
	
	size_t buff_size = BUFF_INCR;
	size_t pointer_pos = 0;
	size_t line_length = 0;
	char *line_pointer = (char *) error_checked_malloc(BUFF_INCR); // Static buffer.
	char *line_buffer = (char *) error_checked_malloc(BUFF_INCR); // Resizable buffer.
	
	for(;;) {
		if (fgets(line_pointer, BUFF_INCR, file_pointer) == NULL) return NULL; // EOF.
		
		line_length = strlen(line_pointer);
		
		if ( line_length < BUFF_INCR-1 ) break; // End of line.
				
		// Increase the size of the buffer and copy contents.
		if (pointer_pos < LINE_LIMIT) {
			line_buffer = (char *) error_checked_realloc(line_buffer, (buff_size+=BUFF_INCR)); // Increase buffer size by BUFF_INCR (128).
			strncpy(line_buffer+pointer_pos, line_pointer, BUFF_INCR-1); // Copy contents from line into buffer.
			pointer_pos += BUFF_INCR-1; // Move write pointer position to be at the start of the 'next' buffer segment.
		} else 
		{
			fprintf(stderr, "Error whilst parsing line into buffer of size %u.\n", buff_size);
			exit(0);
		}
	}
	
	strncpy(line_buffer+pointer_pos, line_pointer, line_length);
	line_buffer[pointer_pos+line_length-1] = '\0';
	free(line_pointer);
	return line_buffer;
}

#endif /* FILE_H */