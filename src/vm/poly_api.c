#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "poly_config.h"
#include "poly_vm.h"
#include "poly_log.h"

static void *defaultAllocate(void *ptr, size_t size)
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

POLY_API void polyInitConfig(poly_Config *config)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Initializing config...\n")
#endif

	config->alloc = defaultAllocate;
}

POLY_API poly_VM *polyNewVM(poly_Config *config)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Creating new VM...\n")
#endif

	poly_Allocator alloc = defaultAllocate;

	if (config != NULL)
		alloc = config->alloc;

	poly_VM* vm = (poly_VM*)alloc(NULL, sizeof(poly_VM));
	vm->config = (poly_Config*)alloc(NULL, sizeof(poly_Config));

	if (config == NULL)
		polyInitConfig(vm->config);
	else
	{
		alloc(vm->config, 0);
		memcpy(vm->config, config, sizeof(poly_Config));
	}

	poly_TokenStream *tokenstream = &vm->parser.tokenstream;
	tokenstream->allotedmem = tokenstream->size = 0;
	tokenstream->maxmem = POLY_INIT_MEM;
	tokenstream->stream = vm->config->alloc(NULL, POLY_INIT_MEM);
	tokenstream->cur = tokenstream->stream;

	poly_CodeStream *codestream = &vm->codestream;
	codestream->allotedmem = codestream->size = 0;
	codestream->maxmem = POLY_INIT_MEM;
	codestream->stream = vm->config->alloc(NULL, POLY_INIT_MEM);
	codestream->cur = codestream->stream;

	return vm;
}

POLY_API void polyFreeVM(poly_VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Freeing VM...\n")
#endif

	vm->config->alloc(vm->parser.tokenstream.stream, 0);
	vm->config->alloc(vm->codestream.stream, 0);
	// We use the default allocator because we need to deallocate the config
	// and the VM
	defaultAllocate(vm->config, 0);
	defaultAllocate(vm, 0);
}

POLY_API void polyInterpret(poly_VM *vm, const char *src)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(API, "Interpreting source...\n")
#endif

	vm->parser.lexer.src = vm->parser.lexer.curchar = src;

	lex(vm);
	parse(vm);
	interpret(vm);
}
