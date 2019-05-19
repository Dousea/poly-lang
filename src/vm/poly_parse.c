#include <stdio.h>
#include <assert.h>

#include "poly_vm.h"
#include "poly_log.h"

static const Token* curtoken(Parser *parser)
{
	return parser->curtoken;
}

// Advances to the next token
static void advtoken(Parser *parser)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Consuming token 0x%02X...\n", parser->curtoken->type)
#endif

	parser->curtoken++;
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

/***** GRAMMAR RULES *****/

static _Bool value(VM *vm)
{
	switch (curtoken(&vm->parser)->type)
	{
	case TOKEN_FALSE:
	case TOKEN_TRUE:
	case TOKEN_NUMBER:
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got literal.\n")
#endif

		advtoken(&vm->parser);
		return 1;
	case TOKEN_IDENTIFIER:
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got identifier.\n")
#endif

		advtoken(&vm->parser);
		return 1;
	default:
		break;
	}

	return 0;
}

static _Bool binaryoperator(VM *vm)
{
	switch (curtoken(&vm->parser)->type)
	{
	case TOKEN_PLUS:
	case TOKEN_MINUS:
	case TOKEN_ASTERISK:
	case TOKEN_SLASH:
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got binary operator.\n")
#endif

		advtoken(&vm->parser);
		return 1;
	default:
		break;
	}

	return 0;
}

static _Bool unaryoperator(VM *vm)
{
	switch (curtoken(&vm->parser)->type)
	{
	case TOKEN_MINUS:
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got unary operator.\n")
#endif

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
		if (binaryoperator(vm))
		{
			if (value(vm) || expression(vm))
			{
#ifdef POLY_DEBUG
				POLY_IMM_LOG(PRS, "Got binary expression.\n")
#endif

				return 1;
			}
		}
		else
		{
#ifdef POLY_DEBUG
			POLY_IMM_LOG(PRS, "Got value.\n")
#endif

			return 1;
		}
		

	if (unaryoperator(vm))
		if (value(vm) || expression(vm))
		{
#ifdef POLY_DEBUG
			POLY_IMM_LOG(PRS, "Got unary expression.\n")
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
		POLY_IMM_LOG(PRS, "Got variable.\n")
#endif

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
		POLY_IMM_LOG(PRS, "Got assignment.\n")
#endif

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
	int curln = 1;

	while (curtoken(&vm->parser)->type != TOKEN_EOF)
	{
		switch (curtoken(&vm->parser)->type)
		{
		case TOKEN_NEWLINE:
			advtoken(&vm->parser);
			curln++;
			break;
		case TOKEN_INDENT:
			advtoken(&vm->parser);
			break;
		case TOKEN_UNKNOWN:
			advtoken(&vm->parser);
			// TODO: Gives a parser error
			fprintf(stderr, "An unknown symbol is found\n"); break;
		default:
			statement(vm);
			break;
		}
	}
}
