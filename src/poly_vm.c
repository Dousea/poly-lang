#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "poly.h"
#include "poly_value.h"
#include "poly_vm.h"

static void push(PolyVM *vm, PolyValue *value)
{
	*(vm->stack + vm->stacksize++) = value;
}

static void pushlitnum(PolyVM *vm, PolyCode *code)
{
	// Convert [size] bytes ahead in [bytecode] so we can transform it to double
	double num;
	size_t size = sizeof(num);
	uint8_t b[size];

	for (int i = 0; i < size; i++)
		b[i] = *(code + i);

	memcpy(&num, &b, size);

	// Create a new PolyValue to be pushed in stack
	PolyValue value;
	value.type = VALUE_NUMBER;
	value.num = num;
	push(vm, &value);
}

static void pushlitbool(PolyVM *vm)
{
	// Just create a new PolyValue as the literal is true or false, no conversion needed
	PolyValue value;
	value.type = VALUE_BOOLEAN;
	value.bool = (CODE_LITERAL_FALSE ? 0 : 1);
	push(vm, &value);
}

static PolyValue *pop(PolyVM *vm)
{
	return *(vm->stack + --vm->stacksize);
}

PolyVM *polyNewVM(void)
{
	PolyVM *vm = (PolyVM*)polyAllocate(NULL, sizeof(PolyVM));
	PolyCompiler *compiler = &vm->compiler;
	PolyParser *parser = &compiler->parser;
	PolyLexer *lexer = &parser->lexer;
	lexer->tokenstart = NULL;
	lexer->stream = polyAllocate(NULL, sizeof(PolyToken));
	lexer->totaltoken = 0;

	return vm;
}

void polyFreeVM(PolyVM *vm)
{
	polyAllocate(vm->compiler.parser.lexer.stream, 0);
//	polyAllocate(vm->stack, 0);
//	polyAllocate(vm->stream, 0);
	polyAllocate(vm, 0);
}

void polyInterpret(PolyVM *vm)
{
	PolyCode *code = vm->stream;

	while (*code != CODE_EOF)
	{
		switch (*code)
		{
		case CODE_LITERAL_NUMBER:
			pushlitnum(vm, code); break;
		case CODE_LITERAL_TRUE: case CODE_LITERAL_FALSE:
			pushlitbool(vm); break;
		default:
			break;
		}

		code++;
	}
}
