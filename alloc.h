#include <stdlib.h>
#include <stdio.h>

void *error_checked_malloc(size_t);
void *error_checked_realloc(void*, size_t);

void *error_checked_malloc(size_t nbytes) {
	void *v_ptr = malloc(nbytes);
	if ( v_ptr == NULL ) {
		fprintf(stderr, "Error whilst allocating %u bytes\n", nbytes);
		exit(0);
	}
	return v_ptr;
}

void *error_checked_realloc(void *ptr, size_t nbytes) {
	ptr = realloc(ptr, nbytes);
	if ( ptr == NULL ) {
		fprintf(stderr, "Error whilst reallocating %u bytes\n", nbytes);
		exit(0);
	}
	return ptr;
}