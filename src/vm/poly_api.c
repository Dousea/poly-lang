#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "poly_config.h"
#include "poly_vm.h"
#include "poly_log.h"

static void *defaultAllocator(void *ptr, size_t size)
{
	if (size == 0)
	{
		free(ptr);

#ifdef POLY_DEBUG
		POLY_IMM_LOG(MEM, "Freed memory at 0x%lX\n", (unsigned long)ptr)
#endif

		return NULL;
	}
	
	void *tmp = realloc(ptr, size);

	if (tmp == NULL)
	{
		if (ptr != NULL)
			free(ptr);

		perror("Memory allocation failed");
		exit(-1);
	}

#ifdef POLY_DEBUG
	POLY_LOG_START(MEM)

	if (ptr == NULL)
		POLY_LOG("Allocated %zu bytes at 0x%lX\n", size, (unsigned long)tmp)
	else if (ptr == tmp)
		POLY_LOG("Allocated %zu bytes to 0x%lX\n", size, (unsigned long)tmp)
	else
		POLY_LOG("Reallocated %zu bytes from 0x%lX to 0x%lX\n", size, (unsigned long)ptr, (unsigned long)tmp)

	POLY_LOG_END
#endif

	return tmp;
}

POLY_API void polyInitConfig(Config *config)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Initializing config...\n")
#endif

	config->allocator = defaultAllocator;
}

POLY_API VM *polyNewVM(Config *config)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Creating new VM...\n")
#endif

	Allocator allocate = defaultAllocator;

	if (config != NULL)
		allocate = config->allocator;

	VM *vm = (VM*)allocate(NULL, sizeof(VM));
	vm->config = (Config*)allocate(NULL, sizeof(Config));

	if (config == NULL)
		polyInitConfig(vm->config);
	else
	{
		allocate(vm->config, 0);
		memcpy(vm->config, config, sizeof(Config));
	}

	vm->parser.allocatedmemory = 0;
	vm->parser.maxmemory = POLY_INITIAL_MEM;
	vm->parser.tokenstream = vm->config->allocator(NULL, POLY_INITIAL_MEM);
	vm->parser.totaltoken = 0;

	return vm;
}

POLY_API void polyFreeVM(VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Freeing VM...\n")
#endif

	vm->config->allocator(vm->parser.tokenstream, 0);
	// We use the default allocator because we need to deallocate the config
	// and the VM
	defaultAllocator(vm->config, 0);
	defaultAllocator(vm, 0);
}

/*
static void push(VM *vm, Value *value)
{
	assert(vm->stacksize < POLY_MAX_STACK);

	vm->stack[vm->stacksize++] = value;
}

static Value *pop(VM *vm)
{
	assert(vm->stacksize > 0);

	return vm->stack[--vm->stacksize];
}
*/

POLY_API void polyInterpret(VM *vm, const char *source)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Interpreting `source`...\n")
#endif

	vm->parser.lexer.source = vm->parser.lexer.curchar = source;

	lex(vm);
	parse(vm);

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
