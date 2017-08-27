#ifndef POLY_H
#define POLY_H

// Maximum values inside stack of VM
#define POLY_MAX_STACK  		128
// Maximum scopes
#define POLY_MAX_SCOPES 		8
// Maximum locals inside a scope level
#define POLY_MAX_LOCALS 		256
// Initial heap for memory allocation in bytes
#define POLY_INITIAL_MEM		1024
// Expression used on new memory allocation, result in bytes
#define POLY_ALLOCATE_MEM(x)	x * 2

typedef struct Config    PolyConfig;
typedef struct Allocator PolyAllocator;
typedef struct VM        PolyVM;

#endif
