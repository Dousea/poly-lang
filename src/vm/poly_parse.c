#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "poly_vm.h"
#include "poly_code.h"
#include "poly_log.h"

static void throwerr(Parser* parser, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "\x1B[1;Parsing error: line %zu: ", parser->curln);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\x1B[0m");
	va_end(args);
	exit(EXIT_FAILURE);
}

// Gets current token that's being parsed
static const Token* curtoken(Parser *parser)
{
	return parser->tokenstream.current;
}

// Advances to the next token
static void advtoken(Parser *parser)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(MEM, "0x%lX: consuming token 0x%02X...\n",
		(unsigned long)parser->tokenstream.current,
		parser->tokenstream.current->type)
#endif

	parser->tokenstream.current++;
}

// If current token type is [t] advance to the token then return 1, otherwise
// return 0
static _Bool curtokenadv(Parser *parser, TokenType type)
{
	if (curtoken(parser)->type == type)
	{
		advtoken(parser);
		return 1;
	}
	else
		return 0;
}

// Allocates [code] in the codestream
static Code* allocatecode(VM *vm, Code code)
{
	size_t size = sizeof(Code);

	if ((vm->codestream.allocatedmemory + size) > vm->codestream.maxmemory)
	{
		vm->codestream.maxmemory = POLY_ALLOCATE_MEM(vm->codestream.maxmemory);
		vm->codestream.stream = vm->config->allocator(vm->codestream.stream,
		                                               vm->codestream.maxmemory);

#ifdef POLY_DEBUG
		POLY_IMM_LOG(MEM, "Resized code stream memory to %zu bytes\n", vm->codestream.maxmemory)
#endif
	}

	vm->codestream.allocatedmemory += size;
	vm->codestream.stream[++vm->codestream.size - 1] = code;
	
#ifdef POLY_DEBUG
	if (code.type == CODE_INST)
		POLY_IMM_LOG(MEM, "0x%lX: created code instruction 0x%02X\n",
			(unsigned long)(vm->codestream.stream + (vm->codestream.size - 1)),
			code.inst)
	else
		POLY_IMM_LOG(MEM, "0x%lX: created code value 0x%02X\n",
			(unsigned long)(vm->codestream.stream + (vm->codestream.size - 1)),
			code.value->type)
#endif

	return (vm->codestream.stream + (vm->codestream.size - 1));
}

// Creates a new code then put it in codestream
static void mkcode(VM *vm, Instruction inst, const Value* value)
{
	Code code;
	code.type = CODE_INST;
	code.inst = inst;

	allocatecode(vm, code);

	if (inst == INST_LITERAL)
	{
		Code code;
		code.type = CODE_VALUE;
		code.value = value;

		allocatecode(vm, code);
	}
}

/***** GRAMMAR RULES *****/

static _Bool value(VM *vm)
{
	switch (curtoken(&vm->parser)->type)
	{
	case TOKEN_FALSE:
	case TOKEN_TRUE:
	case TOKEN_NUMBER:
	case TOKEN_IDENTIFIER:
#ifdef POLY_DEBUG
		POLY_LOG_START(PRS)
		POLY_LOG("Got ")

		if (curtoken(&vm->parser)->value.type == VALUE_NUMBER)
			POLY_LOG("number: %e", curtoken(&vm->parser)->value.num)
		else if (curtoken(&vm->parser)->value.type == VALUE_BOOLEAN)
			POLY_LOG("boolean: %s", (curtoken(&vm->parser)->value.bool ? "true" : "false"))
		else
			POLY_LOG("identifier: '%s'", curtoken(&vm->parser)->value.str)
		
		POLY_LOG("\n")
		POLY_LOG_END
#endif
		
		mkcode(vm, INST_LITERAL, &curtoken(&vm->parser)->value);
		advtoken(&vm->parser);
		return 1;
	default:
		break;
	}

	return 0;
}

static TokenType binaryoperator(VM *vm)
{
	TokenType type = curtoken(&vm->parser)->type;

	switch (type)
	{
	case TOKEN_PLUS:
	case TOKEN_MINUS:
	case TOKEN_ASTERISK:
	case TOKEN_SLASH:
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got binary operator\n")
#endif
		advtoken(&vm->parser);
		return type;
	default:
		break;
	}
	
	return TOKEN_EOF;
}

static _Bool unaryoperator(VM *vm)
{
	switch (curtoken(&vm->parser)->type)
	{
	case TOKEN_MINUS:
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got unary operator\n")
#endif

		mkcode(vm, INST_UN_NEG, NULL);
		advtoken(&vm->parser);

		return 1;
	default:
		break;
	}

	return 0;
}

static _Bool expression(VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading expression...\n")
#endif

	if (value(vm) || expression(vm))
	{
		TokenType type = binaryoperator(vm);

		if (type != TOKEN_EOF)
		{
			if (expression(vm))
			{
#ifdef POLY_DEBUG
				POLY_IMM_LOG(PRS, "Got binary expression\n")
#endif
				
				if (type == TOKEN_PLUS)
					mkcode(vm, INST_BIN_ADD, NULL);
				else if (type == TOKEN_MINUS)
					mkcode(vm, INST_BIN_SUB, NULL);
				else if (type == TOKEN_ASTERISK)
					mkcode(vm, INST_BIN_MUL, NULL);
				else
					mkcode(vm, INST_BIN_DIV, NULL);

				return 1;
			}
		}
		else
			// Here we got a value or an expression, but no binary operator...
			// it's still a valid expression.
			return 1;
	}

	if (unaryoperator(vm))
		if (value(vm) || expression(vm))
		{
#ifdef POLY_DEBUG
			POLY_IMM_LOG(PRS, "Got unary expression\n")
#endif

			return 1;
		}

	return 0;
}

static _Bool expressionlist(VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading expression list...\n")
#endif

	if (expression(vm))
	{
		while (curtokenadv(&vm->parser, TOKEN_COMMA))
			if (expression(vm))
				continue;
			else
				return 0;
		
		return 1;
	}

	return 0;
}

static _Bool variable(VM *vm)
{
	if (curtoken(&vm->parser)->type == TOKEN_IDENTIFIER)
	{
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got '%s' variable\n", curtoken(&vm->parser)->value.str)
#endif
		mkcode(vm, INST_LITERAL, &curtoken(&vm->parser)->value);
		advtoken(&vm->parser);

		return 1;
	}
	
	return 0;
}

static _Bool variablelist(VM *vm)
{	
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading variable list...\n")
#endif

	if (variable(vm))
	{
		while (curtokenadv(&vm->parser, TOKEN_COMMA))
			if (variable(vm))
				continue;
			else
				return 0;
		
		return 1;
	}

	return 0;
}

static _Bool statement(VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading statement...\n")
#endif

	if (variablelist(vm) &&
	    curtokenadv(&vm->parser, TOKEN_EQ) &&
		expressionlist(vm))
	{
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got assignment\n")
#endif

		mkcode(vm, INST_ASSIGN, NULL);

		return 1;
	}
	
	return 0;
}

// Checks if the lexical token stream is at an allowable form and creates bytecodes
POLY_LOCAL void parse(VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Parsing...\n")
#endif

	// Current line position of token that's being consumed
	vm->parser.curln = 1;

	while (curtoken(&vm->parser)->type != TOKEN_EOF)
	{
		switch (curtoken(&vm->parser)->type)
		{
		case TOKEN_NEWLINE:
			advtoken(&vm->parser);
			vm->parser.curln++;
			break;
		case TOKEN_INDENT:
			advtoken(&vm->parser);
			break;
		case TOKEN_UNKNOWN:
			advtoken(&vm->parser);
			throwerr(&vm->parser, "unknown symbol");
			break;
		default:
			statement(vm);
			break;
		}
	}
	
	mkcode(vm, INST_END, NULL);

#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Allocated %zu bytes for %zu codes\n",
		vm->codestream.allocatedmemory,
		vm->codestream.size)
#endif
}
