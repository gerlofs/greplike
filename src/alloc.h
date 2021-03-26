/* GREPLIKE */
/* ALLOC.H */

#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>
#include <stdio.h>
// bit manipulation macros:
#define is_bit_set(val, posn) ((val) & (1 << (posn)))
#define set_bit(val, n) ((val) | (1UL << (n)))

extern void *error_checked_malloc(size_t);
extern void *error_checked_realloc(void*, size_t);

#endif /* ALLOC_H */
