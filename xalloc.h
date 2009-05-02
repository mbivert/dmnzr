#ifndef H_ALLOC
#define H_ALLOC

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void *xmalloc (size_t);
void *xcalloc (size_t, size_t);

#define xfree(p) (free(p), p = NULL)

#endif /* H_ALLOC (guard) */

