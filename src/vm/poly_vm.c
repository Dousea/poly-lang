#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "poly_vm.h"
#include "poly_value.h"
#include "poly_log.h"

static void throwerr(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "\x1B[1;31mError: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\x1B[0m");
	va_end(args);
	exit(EXIT_FAILURE);
}

static void pushvalue(VM* vm, const Value* value)
{
	assert(vm->stack.size < POLY_MAX_STACK);

#ifdef POLY_DEBUG
	POLY_LOG_START(VMA)
	POLY_LOG("Pushing ")

	if (value->type == VALUE_NUMBER)
		POLY_LOG("number: %e", value->num)
	else if (value->type == VALUE_BOOLEAN)
		POLY_LOG("boolean: %s", (value->bool ? "true" : "false"))
	else
		POLY_LOG("identifier: '%s'", value->str)
	
	POLY_LOG("...\n")
	POLY_LOG_END
#endif

	vm->stack.value[vm->stack.size++] = value;
}

static const Value* popvalue(VM* vm)
{
	assert(vm->stack.size > 0);
	
	const Value* value = vm->stack.value[--vm->stack.size];

#ifdef POLY_DEBUG
	POLY_LOG_START(VMA)
	POLY_LOG("Popping ")

	if (value->type == VALUE_NUMBER)
		POLY_LOG("number: %e", value->num)
	else if (value->type == VALUE_BOOLEAN)
		POLY_LOG("boolean: %s", (value->bool ? "true" : "false"))
	else
		POLY_LOG("identifier: '%s'", value->str)
	
	POLY_LOG("...\n")
	POLY_LOG_END
#endif

	vm->stack.value[vm->stack.size] = NULL;
	return value;
}

long hash(const char* str)
{
    long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return hash;
}

long localindex(const char* str)
{
	// FIXME: What should I do when `index` is actually in collision?
	return hash(str) % POLY_MAX_LOCALS;
}

static const Value* getvalue(VM* vm, const char* identifier)
{
	long index = localindex(identifier);
#ifdef POLY_DEBUG
	POLY_IMM_LOG(VMA, "Reading local '%s' from index %ld...\n", identifier, index)
#endif
	Scope* scope = vm->scope[vm->curscope];
	Variable* local = scope->locals[index];
	return local->value;
}

static void addlocal(VM* vm, const char* identifier, const Value* value)
{
	Variable* local = vm->config->allocator(NULL, sizeof(Variable*));
	local->identifier = identifier;
	local->value = value;

	long index = localindex(local->identifier);
	Scope* scope = vm->scope[vm->curscope];
	scope->locals[index] = local;

#ifdef POLY_DEBUG
	POLY_IMM_LOG(VMA, "Added local '%s' to index %ld\n", local->identifier, index)
#endif
}

static const Code* curcode(VM* vm)
{
	return vm->codestream.current;
}

static void advcode(VM* vm)
{
	vm->codestream.current++;
}

POLY_LOCAL void interpret(VM* vm)
{
	Scope* scope = vm->config->allocator(NULL, sizeof(Scope*));
	vm->scope[vm->curscope] = scope;

	while (curcode(vm)->inst != INST_END)
	{
#ifdef POLY_DEBUG
		POLY_IMM_LOG(VMA, "Reading instruction 0x%02X...\n", curcode(vm)->inst)
#endif
		switch (curcode(vm)->inst)
		{
		case INST_LITERAL:
		{
			advcode(vm);
			pushvalue(vm, curcode(vm)->value);

			break;
		}
		case INST_GET_VALUE:
		{
			const Value* identifier = popvalue(vm);

			if (identifier->type == VALUE_IDENTIFIER)
			{
				const Value* value = getvalue(vm, identifier->str);
				pushvalue(vm, value);
			}
			else
				throwerr("identifier expected");

			break;
		}
		case INST_BIN_ADD:
		{
			const Value* rightvalue = popvalue(vm);
			const Value* leftvalue = popvalue(vm);

			if (rightvalue->type == VALUE_IDENTIFIER)
				rightvalue = getvalue(vm, rightvalue->str);
			if (leftvalue->type == VALUE_IDENTIFIER)
				leftvalue = getvalue(vm, leftvalue->str);

			Value newvalue;
			
			if (leftvalue->type == VALUE_NUMBER &&
			    rightvalue->type == VALUE_NUMBER)
			{
				newvalue.type = VALUE_NUMBER;
				newvalue.num = leftvalue->num + rightvalue->num;
			}
			else
				throwerr("addition of the operands is illegal");

			pushvalue(vm, &newvalue);

			break;
		}
		case INST_ASSIGN:
		{
			const Value* value = popvalue(vm);
			const Value* identifier = popvalue(vm);

			if (identifier->type == VALUE_IDENTIFIER)
				addlocal(vm, identifier->str, value);
			else
				throwerr("identifier expected");
			
			break;
		}
		default:
			break;
		}

		advcode(vm);
	}
}