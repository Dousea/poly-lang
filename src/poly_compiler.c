#include <stdio.h>

#include "poly.h"
#include "poly_compiler.h"

// Compiles [source] into readable bytecodes to VM
void polyCompile(PolyCompiler *compiler, const char *source)
{
#ifdef POLY_DEBUG
	printf("Compiling \"%s\" with compiler:%lu\n", source, (unsigned long)compiler);
#endif

	PolyParser *parser = &compiler->parser;
	PolyLexer *lexer = &parser->lexer;
	lexer->source = source;
	lexer->curchar = lexer->source;

	polyLex(lexer);
	polyParse(parser);
	// TODO: Where's the compilation?
}
