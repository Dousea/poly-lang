#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "poly.h"
#include "poly_value.h"
#include "poly_parser.h"
#include "poly_vm.h"

// Hashes string [s]
static uint32_t hash(char *s)
{
	uint32_t hashval;

	for (hashval = 0; *s != '\0'; s++)
		hashval = *s + 31 * hashval;

	return hashval % MAX_LOCALS;
}

// Looks up PolyLocal with name [s] inside [locals]
static Local *lookup(Local *locals, char *s)
{
	Local *local;

	for (local = &locals[hash(s)]; local != NULL; local = local->next)
		if (strcmp(s, local->name) == 0)
			return local;

	return NULL;
}

// Inserts PolyLocal with name [name] and value [value] to [locals]
static Local *insert(Local *locals, char *name, Value value)
{
	Local *local;

	// Not found? Insert it right away!
	if ((local = lookup(locals, name)) == NULL)
	{
		local = (Local*)polyAllocate(NULL, sizeof(Local));

		// Can't duplicate the name? Then nothing we can do
		if ((local->name = strdup(name)) == NULL)
			return NULL;

		uint32_t hashval = hash(name);
		local->next = &locals[hashval];
		locals[hashval] = *local;
	}
	else
		free(local->value);

	// Duplicate [value]
	/*
	PolyValue *cpy = (PolyValue*)malloc(sizeof(PolyValue));

	if (cpy != NULL)
		memcpy(cpy, value, sizeof(*cpy));
	else
		return NULL;
	*/

	local->value = &value; // local->value = cpy

	return local;
}

static void push(PolyVM *vm, Value *value)
{
	*(vm->stack + vm->stacksize++) = value;
}

static void pushlitnum(PolyVM *vm, Code *code)
{
	// Convert [size] bytes ahead in [bytecode] so we can transform it to double
	double num;
	size_t size = sizeof(num);
	uint8_t b[size];

	for (int i = 0; i < size; i++)
		b[i] = *(code + i);

	memcpy(&num, &b, size);

	// Create a new PolyValue to be pushed in stack
	Value value;
	value.type = VALUE_NUMBER;
	value.num = num;
	push(vm, &value);
}

static void pushlitbool(PolyVM *vm)
{
	// Just create a new PolyValue as the literal is true or false, no conversion needed
	Value value;
	value.type = VALUE_BOOLEAN;
	value.bool = (CODE_LITERAL_FALSE ? 0 : 1);
	push(vm, &value);
}

static Value *pop(PolyVM *vm)
{
	return *(vm->stack + --vm->stacksize);
}

PolyVM *polyNewVM(void)
{
	PolyVM *vm = (PolyVM*)polyAllocate(NULL, sizeof(PolyVM));
	Parser *parser = &vm->parser;
	parser->tokenstart = NULL;
	parser->tokenstream = polyAllocate(NULL, sizeof(Token));
	parser->totaltoken = 0;
	// TODO: Allocation for parser->codestream?

	return vm;
}

void polyFreeVM(PolyVM *vm)
{
	polyAllocate(vm->parser.tokenstream, 0);
//	polyAllocate(vm->parser.codestream, 0);
//	polyAllocate(vm->stack, 0);
	polyAllocate(vm, 0);
}

void polyInterpret(PolyVM *vm, const char *source)
{
	Parser *parser = &vm->parser;

	polyParse(parser, source);

	/*
	PolyCode *code = parser->codestream;

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
	*/
}
