#ifndef POLY_PARSER_H_
#define POLY_PARSER_H_

#include "poly_value.h"

typedef struct
{
	// Source that's being lexed
	const char *source;
	// Current lexed character that points to [source]
	const char *curchar;
	// Character that points to [source] of the new token's start
	const char *tokenstart;
	// Lexical token stream that's used by parser
	PolyToken *tokenstream;
	// Total allocated tokens in [stream]
	int totaltoken;
	// Bytecode stream that's interpreted by the VM
	PolyCode *codestream;
} PolyParser;

void polyParse(PolyParser *parser, const char *source);

#endif /* POLY_PARSER_H_ */
