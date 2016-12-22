#ifndef POLY_H_
#define POLY_H_

#define POLY_DEBUG

// This will be the default memory-management function (allocating and freeing)
// If [size] > 0 then it will try to allocate [size] bytes of memory to [ptr]
// Otherwise it will try to free memory pointed to by [ptr]
void *polyAllocate(void *ptr, size_t size);

// TODO: Use this if configuration already implemented
// typedef void* (*PolyAllocate)(void *ptr, size_t size);

#endif
