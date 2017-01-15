#ifndef SRC_VM_POLY_PARSE_H_
#define SRC_VM_POLY_PARSE_H_

#include "poly_lex.h"

typedef struct
{
	Lexer lexer;
	Token *tokenstream;
	int totaltoken;
} Parser;

#endif
