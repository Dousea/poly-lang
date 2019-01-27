#ifndef POLY_CONFIG_H
#define POLY_CONFIG_H

#include <stdlib.h>

typedef void* (*Allocator)(void *ptr, size_t size);

typedef struct
{
	Allocator allocator;
} Config;

#endif
