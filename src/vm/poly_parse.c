#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "poly_vm.h"
#include "poly_code.h"
#include "poly_log.h"

static void throwerr(poly_Parser *parser, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "\x1B[1;Parsing error: line %zu: ", parser->curln);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\x1B[0m\n");
	va_end(args);
	exit(EXIT_FAILURE);
}

// Gets current token that's being parsed
static const poly_Token *curtoken(poly_Parser *parser)
{
	return parser->tokenstream.cur;
}

// Advances to the next token
static void advtoken(poly_Parser *parser)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(MEM, "0x%lX: consuming token 0x%02X...\n",
		(unsigned long)parser->tokenstream.cur,
		parser->tokenstream.cur->type)
#endif

	parser->tokenstream.cur++;
}

// If current token type is [t] advance to the token then return 1, otherwise
// return 0
static _Bool curtokenadv(poly_Parser *parser, poly_TokenType type)
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
static poly_Code *alloccode(poly_VM *vm, poly_Code code)
{
	size_t size = sizeof(poly_Code);

	if ((vm->codestream.allotedmem + size) > vm->codestream.maxmem)
	{
		vm->codestream.maxmem = POLY_ALLOC_MEM(vm->codestream.maxmem);
		vm->codestream.stream = vm->config->alloc(vm->codestream.stream,
		                                          vm->codestream.maxmem);

#ifdef POLY_DEBUG
		POLY_IMM_LOG(MEM, "Resized code stream memory to %zu bytes\n", vm->codestream.maxmem)
#endif
	}

	vm->codestream.allotedmem += size;
	vm->codestream.stream[++vm->codestream.size - 1] = code;
	
#ifdef POLY_DEBUG
	if (code.type == POLY_CODE_INST)
		POLY_IMM_LOG(MEM, "0x%lX: created code instruction 0x%02X\n",
			(unsigned long)(vm->codestream.stream + (vm->codestream.size - 1)),
			code.inst)
	else
		POLY_IMM_LOG(MEM, "0x%lX: created code value 0x%02X\n",
			(unsigned long)(vm->codestream.stream + (vm->codestream.size - 1)),
			code.val->type)
#endif

	return (vm->codestream.stream + (vm->codestream.size - 1));
}

// Creates a new code then put it in codestream
static void mkcode(poly_VM *vm, poly_Instruction inst, const poly_Value *val)
{
	poly_Code code;
	code.type = POLY_CODE_INST;
	code.inst = inst;

	alloccode(vm, code);

	if (inst == POLY_INST_LITERAL)
	{
		poly_Code code;
		code.type = POLY_CODE_VALUE;
		code.val = val;

		alloccode(vm, code);
	}
}

/**** *GRAMMAR RULES *****/

static _Bool value(poly_VM *vm)
{
	switch (curtoken(&vm->parser)->type)
	{
	case POLY_TOKEN_FALSE:
	case POLY_TOKEN_TRUE:
	case POLY_TOKEN_NUMBER:
	case POLY_TOKEN_IDENTIFIER:
#ifdef POLY_DEBUG
		POLY_LOG_START(PRS)
		POLY_LOG("Got ")

		if (curtoken(&vm->parser)->val.type == POLY_VAL_NUM)
			POLY_LOG("number: %e", curtoken(&vm->parser)->val.num)
		else if (curtoken(&vm->parser)->val.type == POLY_VAL_BOOL)
			POLY_LOG("boolean: %s", (curtoken(&vm->parser)->val.bool ? "true" : "false"))
		else
			POLY_LOG("identifier: '%s'", curtoken(&vm->parser)->val.str)
		
		POLY_LOG("\n")
		POLY_LOG_END
#endif
		
		mkcode(vm, POLY_INST_LITERAL, &curtoken(&vm->parser)->val);
		advtoken(&vm->parser);
		return 1;
	default:
		break;
	}

	return 0;
}

static poly_TokenType binaryoperator(poly_VM *vm)
{
	poly_TokenType type = curtoken(&vm->parser)->type;

	switch (type)
	{
	case POLY_TOKEN_PLUS:
	case POLY_TOKEN_MINUS:
	case POLY_TOKEN_ASTERISK:
	case POLY_TOKEN_SLASH:
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got binary operator\n")
#endif
		advtoken(&vm->parser);
		return type;
	default:
		break;
	}
	
	return POLY_TOKEN_EOF;
}

static _Bool unaryoperator(poly_VM *vm)
{
	switch (curtoken(&vm->parser)->type)
	{
	case POLY_TOKEN_MINUS:
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got unary operator\n")
#endif

		mkcode(vm, POLY_INST_UN_NEG, NULL);
		advtoken(&vm->parser);

		return 1;
	default:
		break;
	}

	return 0;
}

static _Bool expression(poly_VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading expression...\n")
#endif

	if (value(vm) || expression(vm))
	{
		poly_TokenType type = binaryoperator(vm);

		if (type != POLY_TOKEN_EOF)
		{
			if (expression(vm))
			{
#ifdef POLY_DEBUG
				POLY_IMM_LOG(PRS, "Got binary expression\n")
#endif
				
				if (type == POLY_TOKEN_PLUS)
					mkcode(vm, POLY_INST_BIN_ADD, NULL);
				else if (type == POLY_TOKEN_MINUS)
					mkcode(vm, POLY_INST_BIN_SUB, NULL);
				else if (type == POLY_TOKEN_ASTERISK)
					mkcode(vm, POLY_INST_BIN_MUL, NULL);
				else
					mkcode(vm, POLY_INST_BIN_DIV, NULL);

				return 1;
			}
		}
		else
			// Here we got a val or an expression, but no binary operator...
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

static _Bool expressionlist(poly_VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading expression list...\n")
#endif

	if (expression(vm))
	{
		while (curtokenadv(&vm->parser, POLY_TOKEN_COMMA))
			if (expression(vm))
				continue;
			else
				return 0;
		
		return 1;
	}

	return 0;
}

static _Bool variable(poly_VM *vm)
{
	if (curtoken(&vm->parser)->type == POLY_TOKEN_IDENTIFIER)
	{
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got '%s' variable\n", curtoken(&vm->parser)->val.str)
#endif
		mkcode(vm, POLY_INST_LITERAL, &curtoken(&vm->parser)->val);
		advtoken(&vm->parser);

		return 1;
	}
	
	return 0;
}

static _Bool variablelist(poly_VM *vm)
{	
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading variable list...\n")
#endif

	if (variable(vm))
	{
		while (curtokenadv(&vm->parser, POLY_TOKEN_COMMA))
			if (variable(vm))
				continue;
			else
				return 0;
		
		return 1;
	}

	return 0;
}

static _Bool statement(poly_VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading statement...\n")
#endif

	if (variablelist(vm) &&
	    curtokenadv(&vm->parser, POLY_TOKEN_EQ) &&
		expressionlist(vm))
	{
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got assignment\n")
#endif

		mkcode(vm, POLY_INST_ASSIGN, NULL);

		return 1;
	}
	
	return 0;
}

// Checks if the lexical token stream is at an allowable form and creates bytecodes
POLY_LOCAL void parse(poly_VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Parsing...\n")
#endif

	// Current line position of token that's being consumed
	vm->parser.curln = 1;

	while (curtoken(&vm->parser)->type != POLY_TOKEN_EOF)
	{
		switch (curtoken(&vm->parser)->type)
		{
		case POLY_TOKEN_NEWLINE:
			advtoken(&vm->parser);
			vm->parser.curln++;
			break;
		case POLY_TOKEN_INDENT:
			advtoken(&vm->parser);
			break;
		case POLY_TOKEN_UNKNOWN:
			advtoken(&vm->parser);
			throwerr(&vm->parser, "unknown symbol");
			break;
		default:
			statement(vm);
			break;
		}
	}
	
	mkcode(vm, POLY_INST_END, NULL);

#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Allocated %zu bytes for %zu codes\n",
		vm->codestream.allotedmem,
		vm->codestream.size)
#endif
}
