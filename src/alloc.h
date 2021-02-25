#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>
#include <stdio.h>

extern void *error_checked_malloc(size_t);
extern void *error_checked_realloc(void*, size_t);

#endif /* ALLOC_H */