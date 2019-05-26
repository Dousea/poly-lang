#ifndef POLY_VM_H
#define POLY_VM_H

#include "poly_config.h"
#include "poly_lex.h"
#include "poly_parse.h"
#include "poly_code.h"

#if defined _WIN32 || defined __CYGWIN__
	#define POLY_HELPER_DLL_IMPORT __declspec(dllimport)
 	#define POLY_HELPER_DLL_EXPORT __declspec(dllexport)
	#define POLY_HELPER_DLL_LOCAL
#else
	#if __GNUC__ >= 4
		#define POLY_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
		#define POLY_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    	#define POLY_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
	#else
    	#define POLY_HELPER_DLL_IMPORT
    	#define POLY_HELPER_DLL_EXPORT
    	#define POLY_HELPER_DLL_LOCAL
  	#endif // __GNUC__
#endif // _WIN32 || __CYGWIN__

#ifdef POLY_DLL
	#ifdef POLY_DLL_EXPORTS // defined if we are building the Poly DLL (instead of using it)
		#define POLY_API POLY_HELPER_DLL_EXPORT
	#else
		#define POLY_API POLY_HELPER_DLL_IMPORT
	#endif // POLY_DLL_EXPORTS
	#define POLY_LOCAL POLY_HELPER_DLL_LOCAL
#else
	#define POLY_API
	#define POLY_LOCAL
#endif // POLY_DLL

// Maximum values inside stack of VM
#define POLY_MAX_STACK  	128
// Maximum scopes
#define POLY_MAX_SCOPES 	8
// Maximum locals inside a scope level
#define POLY_MAX_LOCALS 	256
// Initial heap for memory allocation in bytes
#define POLY_INIT_MEM		1024
// Expression used on new memory allocation, result in bytes
#define POLY_ALLOC_MEM(x)	x * 2

typedef struct poly_Scope
{
	poly_Variable *local[POLY_MAX_LOCALS];
} poly_Scope;

typedef struct poly_Stack
{
	size_t size;
	poly_Value *val[POLY_MAX_STACK];
	const poly_Value *cur;
} poly_Stack;

typedef struct poly_CodeStream
{
	poly_Code *stream;
	const poly_Code *cur;
	size_t allotedmem;
	size_t maxmem;
	size_t size;
} poly_CodeStream;

typedef struct poly_VM
{
	poly_Config *config;
	poly_Lexer lexer;
	poly_Parser parser;
	poly_Stack stack;
	poly_CodeStream codestream;

	poly_Scope *scope[POLY_MAX_SCOPES];
	unsigned int curscope;
} poly_VM;

void lex(poly_VM *vm);
void parse(poly_VM *vm);
void interpret(poly_VM *vm);

#endif
