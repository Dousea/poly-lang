#include <stdio.h>
#include <assert.h>

#include "poly_vm.h"
#include "poly_log.h"

/*
static void pushvalue(VM *vm, Value *value)
{
	assert(vm->stacksize < POLY_MAX_STACK);

	vm->stack[vm->stacksize++] = value;
}

static Value *popvalue(VM *vm)
{
	assert(vm->stacksize > 0);
	
	Value *value = vm->stack[--vm->stacksize];
	vm->stack[vm->stacksize] = NULL;
	return value;
}

char* tokentostr(VM *vm) {
	Token *token = vm->parser.tokenstream;
	char *str = vm->config->allocator(NULL, sizeof(token->len+1));
	for (int i = 0; i < token->len; ++i)
		str[i] = token->start[i];
	str[token->len] = '\0';
	return str;
}

static void pushidentifier(VM *vm)
{
	Token *token = vm->parser.tokenstream;

	Value *value = vm->config->allocator(NULL, sizeof(Value*));
	value->str = tokentostr(vm);
	value->type = VALUE_IDENTIFIER;

	pushvalue(vm, value);
}

static void pushnumber(VM *vm)
{
	// The lexing has taken care of the string to value conversion
	pushvalue(vm, &vm->parser.tokenstream->value);
}

long hash(char *str)
{
    long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return hash;
}

static void addlocal(VM *vm)
{
	Variable *local = vm->config->allocator(NULL, sizeof(Variable*));
	// The stack will be like this: [.., `identifier`, `value`]
	// First pop will be `value`, and second will be `identifier`
	local->value = popvalue(vm);
	local->identifier = popvalue(vm)->str;

	long index = hash(local->identifier) % POLY_MAX_LOCALS;
	Scope *scope = vm->scope[vm->curscope];
	// FIXME: What should I do when `index` is actually in collision?
	scope->locals[index] = local;
}
*/

POLY_LOCAL void interpret(VM *vm)
{
    // TODO: Read the instructions accordingly
}