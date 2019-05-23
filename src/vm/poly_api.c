#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "poly_config.h"
#include "poly_vm.h"
#include "poly_log.h"

static void* defaultAllocator(void* ptr, size_t size)
{
	if (size == 0)
	{
		free(ptr);

#ifdef POLY_DEBUG
		POLY_IMM_LOG(MEM, "0x%lX: freed memory\n", (unsigned long)ptr)
#endif

		return NULL;
	}
	
	void* tmp = realloc(ptr, size);

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
		POLY_LOG("0x%lX: %zu bytes allocated\n", (unsigned long)tmp, size)
	else if (ptr == tmp)
		POLY_LOG("0x%lX: allocated %zu bytes\n", (unsigned long)tmp, size)
	else
		POLY_LOG("0x%lX: from 0x%lX reallocated %zu bytes\n", (unsigned long)tmp, (unsigned long)ptr, size)

	POLY_LOG_END
#endif

	return tmp;
}

POLY_API void polyInitConfig(Config* config)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Initializing config...\n")
#endif

	config->allocator = defaultAllocator;
}

POLY_API VM* polyNewVM(Config* config)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Creating new VM...\n")
#endif

	Allocator allocate = defaultAllocator;

	if (config != NULL)
		allocate = config->allocator;

	VM* vm = (VM*)allocate(NULL, sizeof(VM));
	vm->config = (Config*)allocate(NULL, sizeof(Config));

	if (config == NULL)
		polyInitConfig(vm->config);
	else
	{
		allocate(vm->config, 0);
		memcpy(vm->config, config, sizeof(Config));
	}

	vm->parser.tokenstream.allocatedmemory = 0;
	vm->parser.tokenstream.maxmemory = POLY_INITIAL_MEM;
	vm->parser.tokenstream.stream = vm->config->allocator(NULL, POLY_INITIAL_MEM);
	vm->parser.tokenstream.current = vm->parser.tokenstream.stream;
	vm->parser.tokenstream.size = 0;

	vm->codestream.allocatedmemory = 0;
	vm->codestream.maxmemory = POLY_INITIAL_MEM;
	vm->codestream.stream = vm->config->allocator(NULL, POLY_INITIAL_MEM);
	vm->codestream.current = vm->codestream.stream;
	vm->codestream.size = 0;

	return vm;
}

POLY_API void polyFreeVM(VM* vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Freeing VM...\n")
#endif

	vm->config->allocator(vm->parser.tokenstream.stream, 0);
	vm->config->allocator(vm->codestream.stream, 0);
	// We use the default allocator because we need to deallocate the config
	// and the VM
	defaultAllocator(vm->config, 0);
	defaultAllocator(vm, 0);
}

POLY_API void polyInterpret(VM* vm, const char* source)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Interpreting `source`...\n")
#endif

	vm->parser.lexer.source = vm->parser.lexer.curchar = source;

	lex(vm);
	parse(vm);
	interpret(vm);
}
