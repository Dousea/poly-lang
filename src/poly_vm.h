#ifndef POLY_VM_H_
#define POLY_VM_H_

#include "poly_compiler.h"
#include "poly_value.h"

// Maximum values inside VM's stack
#define MAX_STACK 128

typedef struct
{
	PolyCompiler compiler;
	// Current stack size
	int stacksize;
	// Stack that's used by the VM
	PolyValue *stack[MAX_STACK];
	// Bytecode stream that's interpreted by the VM
	PolyCode *stream;
} PolyVM;

PolyVM *polyNewVM(void);
void polyFreeVM(PolyVM *vm);
void polyInterpret(PolyVM *vm);

#endif /* POLY_VM_H_ */
