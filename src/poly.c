#include <stdlib.h>
#include <stdio.h>

#include "poly.h"

void *polyAllocate(void *ptr, size_t size)
{
	if (size == 0)
	{
		free(ptr);
#ifdef POLY_DEBUG
		printf("memory:%lu freed\n", (unsigned long)ptr);
#endif
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
		printf("Allocated %d bytes at memory:%lu\n", size, (unsigned long)tmp);
	else if (ptr == tmp)
		printf("Allocated %d bytes to memory:%lu\n", size, (unsigned long)tmp);
	else
		printf("Reallocated %d bytes from memory:%lu to memory:%lu\n", size, (unsigned long)ptr, (unsigned long)tmp);
#endif

	return tmp;
}
