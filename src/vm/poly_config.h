#ifndef POLY_CONFIG_H
#define POLY_CONFIG_H

#include <stdlib.h>

typedef void* (*poly_Allocator)(void *ptr, size_t size);

typedef struct poly_Config
{
	poly_Allocator alloc;
} poly_Config;

#endif
