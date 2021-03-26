/* GREPLIKE */
/* ALLOC.C */

/* A set of abstractions that invokes and then checks
 * malloc and realloc. As these need to be flexible,
 * the outputs need to be cast.
 * e.g. char *str = (*char) error_checked_malloc(n);
 */

#include "alloc.h"

void *error_checked_malloc(size_t nbytes) {
	void *v_ptr = malloc(nbytes);
	if ( v_ptr == NULL ) {
		fprintf(stderr, "Error whilst allocating %u bytes\n", nbytes);
		exit(1);
	}
	return v_ptr;
}

void *error_checked_realloc(void *ptr, size_t nbytes) {
	ptr = realloc(ptr, nbytes);
	if ( ptr == NULL ) {
		fprintf(stderr, "Error whilst reallocating %u bytes\n", nbytes);
		exit(1);
	}
	return ptr;
}
