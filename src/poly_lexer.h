#ifndef POLY_LEXER_H_
#define POLY_LEXER_H_

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
	PolyToken *stream;
	// Total allocated tokens in [stream]
	int totaltoken;
} PolyLexer;

void polyLex(PolyLexer *lexer);

#endif /* POLY_LEXER_H_ */
