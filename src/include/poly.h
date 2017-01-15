#ifndef POLY_H
#define POLY_H

// Maximum values inside stack of VM
#define POLY_MAX_STACK  128
// Maximum scopes
#define POLY_MAX_SCOPES 8
// Maximum locals inside a scope level
#define POLY_MAX_LOCALS 256

typedef struct Config    PolyConfig;
typedef struct Allocator PolyAllocator;
typedef struct VM        PolyVM;

#endif
