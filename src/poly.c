#include <stdlib.h>
#include <stdio.h>
#include <math.h> // FIXME: This header only included when POLY_DEBUG is defined

#include "poly.h"

void *polyAllocate(void *ptr, size_t size)
{
	if (size == 0)
	{
#ifdef POLY_DEBUG
		printf("Freeing memory:%lu\n", (unsigned long)ptr);
#endif
		free(ptr);
		return NULL;
	}

	void *tmp = realloc(ptr, size);

	if (tmp == NULL)
	{
		if (ptr != NULL)
			free(ptr);

		perror("Memory allocation failed");
		exit(-1);
	}

#ifdef POLY_DEBUG
	if (ptr == NULL)
		printf("Allocating %d bytes at memory:%lu\n", size, (unsigned long)tmp);
	else if (ptr == tmp)
		printf("Allocating %d bytes to memory:%lu\n", size, (unsigned long)tmp);
	else
		printf("Reallocating from memory:%lu to memory:%lu for %d bytes\n", (unsigned long)ptr, (unsigned long)tmp, size);
#endif

	return tmp;
}
