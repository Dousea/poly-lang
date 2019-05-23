#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "poly_vm.h"
#include "poly_value.h"
#include "poly_log.h"

static void throwerr(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "\x1B[1;31mError: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\x1B[0m\n");
	va_end(args);
	exit(EXIT_FAILURE);
}

static void pushvalue(poly_VM *vm, const poly_Value *val)
{
	assert(vm->stack.size < POLY_MAX_STACK);

#ifdef POLY_DEBUG
	POLY_LOG_START(VMA)
	POLY_LOG("Pushing ")

	if (val->type == POLY_VAL_NUM)
		POLY_LOG("number: %e", val->num)
	else if (val->type == POLY_VAL_BOOL)
		POLY_LOG("boolean: %s", (val->bool ? "true" : "false"))
	else
		POLY_LOG("identifier: '%s'", val->str)
	
	POLY_LOG("...\n")
	POLY_LOG_END
#endif

	vm->stack.val[vm->stack.size++] = val;
}

static const poly_Value *popvalue(poly_VM *vm)
{
	assert(vm->stack.size > 0);
	
	const poly_Value *val = vm->stack.val[--vm->stack.size];

#ifdef POLY_DEBUG
	POLY_LOG_START(VMA)
	POLY_LOG("Popping ")

	if (val->type == POLY_VAL_NUM)
		POLY_LOG("number: %e", val->num)
	else if (val->type == POLY_VAL_BOOL)
		POLY_LOG("boolean: %s", (val->bool ? "true" : "false"))
	else
		POLY_LOG("identifier: '%s'", val->str)
	
	POLY_LOG("...\n")
	POLY_LOG_END
#endif

	vm->stack.val[vm->stack.size] = NULL;
	return val;
}

long hash(const char *str)
{
    long hash = 5381;
    int c;

    while ((c = *str++) != '\0')
        hash = ((hash << 5) + hash) + c; // hash  *33 + c

    return hash;
}

long localindex(const char *str)
{
	// FIXME: What should I do when `index` is actually in collision?
	return hash(str) % POLY_MAX_LOCALS;
}

static const poly_Value *getvalue(poly_VM *vm, const char *id)
{
	long index = localindex(id);
#ifdef POLY_DEBUG
	POLY_IMM_LOG(VMA, "Reading local '%s' from index %ld...\n", id, index)
#endif
	poly_Scope *scope = vm->scope[vm->curscope];
	poly_Variable *local = scope->local[index];
	return local->val;
}

static void addlocal(poly_VM *vm, const char *id, const poly_Value *val)
{
	poly_Variable *local = (poly_Variable*)vm->config->alloc(NULL, sizeof(poly_Variable));
	local->id = id;
	local->val = val;

	long index = localindex(local->id);
	poly_Scope *scope = vm->scope[vm->curscope];
	scope->local[index] = local;

#ifdef POLY_DEBUG
	POLY_IMM_LOG(VMA, "Added local '%s' to index %ld\n", local->id, index)
#endif
}

static const poly_Code *curcode(poly_VM *vm)
{
	return vm->codestream.cur;
}

static void advcode(poly_VM *vm)
{
	vm->codestream.cur++;
}

POLY_LOCAL void interpret(poly_VM *vm)
{
	poly_Scope *scope = vm->config->alloc(NULL, sizeof(poly_Scope*));
	vm->scope[vm->curscope] = scope;

	while (curcode(vm)->inst != POLY_INST_END)
	{
#ifdef POLY_DEBUG
		POLY_IMM_LOG(VMA, "Reading instruction 0x%02X...\n", curcode(vm)->inst)
#endif
		switch (curcode(vm)->inst)
		{
		case POLY_INST_LITERAL:
		{
			advcode(vm);
			pushvalue(vm, curcode(vm)->val);

			break;
		}
		case POLY_INST_GET_VALUE:
		{
			const poly_Value *id = popvalue(vm);

			if (id->type == POLY_VAL_ID)
			{
				const poly_Value *val = getvalue(vm, id->str);
				pushvalue(vm, val);
			}
			else
				throwerr("identifier expected");

			break;
		}
		case POLY_INST_BIN_ADD:
		{
			const poly_Value *rval = popvalue(vm);
			const poly_Value *lval = popvalue(vm);

			// If the values are identifiers, get the respective values
			if (rval->type == POLY_VAL_ID)
				rval = getvalue(vm, rval->str);
			if (lval->type == POLY_VAL_ID)
				lval = getvalue(vm, lval->str);

			poly_Value val;
			
			if (lval->type == POLY_VAL_NUM &&
			    rval->type == POLY_VAL_NUM)
			{
				val.type = POLY_VAL_NUM;
				val.num = lval->num + rval->num;
			}
			else
				throwerr("addition of the operands is illegal");

			pushvalue(vm, &val);

			break;
		}
		case POLY_INST_ASSIGN:
		{
			const poly_Value *val = popvalue(vm);
			const poly_Value *id = popvalue(vm);

			if (id->type == POLY_VAL_ID)
				addlocal(vm, id->str, val);
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