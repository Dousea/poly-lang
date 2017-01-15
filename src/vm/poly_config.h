#ifndef POLY_CONFIG_H
#define POLY_CONFIG_H

#include <stdlib.h>

#if defined(_WIN32)
	#define POLY_API __declspec(dllexport)
#else
	#if __GNUC__ >= 4
		#define POLY_API __attribute__ ((__visibility__ ("default")))
	#else
		#define POLY_API extern
	#endif
#endif

typedef void* (*Allocator)(void *ptr, size_t size);

typedef struct
{
	Allocator allocator;
} Config;

#endif
