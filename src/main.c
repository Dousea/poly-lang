#include <stdlib.h>

#include "compiler.h"

int main()
{
	Compiler *compiler = Compiler_new();

	Compiler_compile(compiler, "a = 12345678e-4");
	Compiler_free(compiler);

	return 0;
}
