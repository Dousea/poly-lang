#ifndef POLY_COMPILER_H_
#define POLY_COMPILER_H_

#include "poly_parser.h"

typedef struct
{
	PolyParser parser;
} PolyCompiler;

void polyCompile(PolyCompiler *compiler, const char *source);

#endif /* POLY_COMPILER_H_ */
