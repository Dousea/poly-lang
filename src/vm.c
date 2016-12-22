#include <stdint.h>
#include <string.h>

#include "value.h"
#include "vm.h"

static void pushlitnum(VM *vm, Instruction *bytecode)
{
	// Convert [size] bytes ahead in [bytecode] so we can transform it to double
	double num;
	size_t size = sizeof(num);
	uint8_t b[size];

	for (int i = 0; i < size; i++)
		b[i] = *(bytecode + i);

	memcpy(&num, &b, size);

	// Create a new Value to be pushed in stack
	Value value;
	value.type = VALUE_NUMBER;
	value.num = num;
	VM_push(vm, &value);
}

static void pushlitbool(VM *vm, Instruction *bytecode)
{
	// Just create a new Value as the literal is true or false, no conversion needed
	Value value;
	value.type = VALUE_BOOLEAN;
	value.bool = (INST_LITERAL_FALSE ? 0 : 1);
	VM_push(vm, &value);
}

void VM_push(VM *vm, Value *value)
{
	vm->stack[vm->stacksize++] = value;
}

Value *VM_pop(VM *vm)
{
	return vm->stack[--vm->stacksize];
}

void VM_interpret(VM *vm, Instruction *bytecode)
{
	while (*bytecode != INST_EOF)
	{
		Instruction *inst = ++bytecode;

		switch (*inst)
		{
		case INST_LITERAL_NUMBER:
			pushlitnum(vm, inst);
			break;
		case INST_LITERAL_TRUE: case INST_LITERAL_FALSE:
			pushlitbool(vm, inst);
			break;
		}
	}
}
