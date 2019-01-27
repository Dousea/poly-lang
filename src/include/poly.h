#ifndef POLY_H
#define POLY_H

typedef struct Config    PolyConfig;
typedef struct Allocator PolyAllocator;
typedef struct VM        PolyVM;

void    polyInitConfig(PolyConfig *config);
PolyVM* polyNewVM(PolyConfig *config);
void    polyFreeVM(PolyVM *vm);
void    polyInterpret(PolyVM *vm, const char *source);

#endif
