#ifndef POLY_PARSER_H_
#define POLY_PARSER_H_

#include "poly_lexer.h"

typedef struct
{
	PolyLexer lexer;
} PolyParser;

void polyParse(PolyParser *parser);

#endif /* POLY_PARSER_H_ */
