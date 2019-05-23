#ifndef POLY_LEX_H
#define POLY_LEX_H

#include "poly_value.h"

typedef struct
{
	const char* source;
	const char* curchar;
	const char* tokenstart;
	size_t curln;
} Lexer;

#endif
