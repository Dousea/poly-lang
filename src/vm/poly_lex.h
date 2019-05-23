#ifndef POLY_LEX_H
#define POLY_LEX_H

#include "poly_value.h"

typedef struct poly_Lexer
{
	const char *src;
	const char *curchar;
	const char *tokenstart;
	size_t curln;
} poly_Lexer;

#endif
