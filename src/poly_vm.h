#ifndef POLY_VM_H_
#define POLY_VM_H_

#include "poly_value.h"
#include "poly_parser.h"

// Maximum values inside VM's stack
#define MAX_STACK 128

typedef struct
{
	PolyParser parser;
	// Current stack size
	int stacksize;
	// Stack that's used by the VM
	PolyValue *stack[MAX_STACK];
} PolyVM;

PolyVM *polyNewVM(void);
void polyFreeVM(PolyVM *vm);
void polyInterpret(PolyVM *vm, const char *source);

#endif /* POLY_VM_H_ */
