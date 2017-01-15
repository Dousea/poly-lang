#ifndef POLY_VM_H
#define POLY_VM_H

#include "poly.h"
#include "poly_config.h"
#include "poly_parse.h"

typedef struct
{
	Config *config;
	Parser parser;
	int stacksize;
	Value *stack[POLY_MAX_STACK];
} VM;

void lex(VM *vm);
void parse(VM *vm, const char *source);

#endif
