#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "poly_config.h"
#include "poly_vm.h"

static void *defaultAllocator(void *ptr, size_t size)
{
	if (size == 0)
	{
		free(ptr);
#ifdef POLY_DEBUG
		printf("memory:%lu freed\n", (unsigned long)ptr);
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
	if (ptr == NULL)
		printf("Allocated %d bytes at memory:%lu\n", size, (unsigned long)tmp);
	else if (ptr == tmp)
		printf("Allocated %d bytes to memory:%lu\n", size, (unsigned long)tmp);
	else
		printf("Reallocated %d bytes from memory:%lu to memory:%lu\n", size, (unsigned long)ptr, (unsigned long)tmp);
#endif

	return tmp;
}

POLY_API void polyInitConfig(Config *config)
{
	config->allocator = defaultAllocator;
}

POLY_API VM *polyNewVM(Config *config)
{
	Allocator allocate = defaultAllocator;

	if (config != NULL)
		allocate = config->allocator;

	VM *vm = (VM*)allocate(NULL, sizeof(VM));
	vm->config = (Config*)allocate(NULL, sizeof(Config));

	if (config == NULL)
	{
		polyInitConfig(vm->config);
	}
	else
	{
		allocate(vm->config, 0);
		memcpy(vm->config, config, sizeof(Config));
	}

	vm->parser.tokenstream = vm->config->allocator(NULL, sizeof(Token));
	vm->parser.totaltoken = 0;

	return vm;
}

POLY_API void polyFreeVM(VM *vm)
{
	vm->config->allocator(vm->parser.tokenstream, 0);
	// We use the default allocator because we need to deallocate the config
	// and the VM
	defaultAllocator(vm->config, 0);
	defaultAllocator(vm, 0);
}

POLY_API void polyInterpret(VM *vm, const char *source)
{
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
