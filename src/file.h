#ifndef FILE_H
#define FILE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "alloc.h"
#define LINE_LIMIT 2048 // Standard UNIX line limit.
#define BUFF_INCR 128 	// Number of bytes to increase the buffer by each time we don't read a line end.

char *read_line(FILE*);
FILE *open_file(char *filename);

#endif /* FILE_H */
