#ifndef POLY_VM_H
#define POLY_VM_H

#include "poly_config.h"
#include "poly_parse.h"

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

typedef struct
{
	Config *config;
	Parser parser;
	unsigned int stacksize;
	Value *stack[POLY_MAX_STACK];
} VM;

void lex(VM *vm);
void parse(VM *vm);

#endif
