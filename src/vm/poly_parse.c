#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "poly_vm.h"
#include "poly_parse.h"
#include "poly_code.h"
#include "poly_log.h"

static void throwerr(poly_Parser *parser, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "\x1B[1;31mParsing error: line %zu: ", parser->curln);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\x1B[0m\n");
	va_end(args);
	exit(EXIT_FAILURE);
}

// Gets current token that's being parsed
static const poly_Token *curtoken(poly_Lexer *lexer)
{
	return lexer->tokenstream.cur;
}

// Gets previous parsed token
static const poly_Token *prevtoken(poly_Lexer *lexer)
{
	return lexer->tokenstream.cur-1;
}

// Advances to the next token
static void advtoken(poly_Lexer *lexer)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(MEM, "0x%lX: consuming token 0x%02X...\n",
		(unsigned long)lexer->tokenstream.cur,
		lexer->tokenstream.cur->type)
#endif

	lexer->tokenstream.cur++;
}

// If current token type is [type] then advance to the token then return 1,
// otherwise return 0
static _Bool curtokenadv(poly_Lexer *lexer, poly_TokenType type)
{
	if (curtoken(lexer)->type == type)
	{
		advtoken(lexer);
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
static void mkcode(poly_VM *vm, poly_Instruction inst, poly_Value *val)
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

inline static _Bool islit(poly_TokenType type)
{
	return (type == POLY_TOKEN_FALSE ||
		    type == POLY_TOKEN_TRUE || 
		    type == POLY_TOKEN_NUMBER ||
		    type == POLY_TOKEN_IDENTIFIER);
}

inline static _Bool isarithop(poly_TokenType type)
{
	return (type == POLY_TOKEN_ASTERISK ||
	        type == POLY_TOKEN_SLASH ||
			type == POLY_TOKEN_PRCNTSGN ||
			type == POLY_TOKEN_PLUS ||
			type == POLY_TOKEN_MINUS);
}

inline static _Bool isrelationop(poly_TokenType type)
{
	return (type == POLY_TOKEN_EQEQ ||
			type == POLY_TOKEN_UNEQ ||
			type == POLY_TOKEN_GTEQ ||
			type == POLY_TOKEN_LTEQ ||
			type == POLY_TOKEN_GT ||
			type == POLY_TOKEN_LT);
}

inline static _Bool islogicop(poly_TokenType type)
{
	return (type == POLY_TOKEN_AND ||
	        type == POLY_TOKEN_OR ||
			type == POLY_TOKEN_NOT);
}

inline static _Bool isop(poly_TokenType type)
{
	return (isarithop(type) ||
	        isrelationop(type) ||
			islogicop(type));
}

static const poly_Operator *gettopopstack(poly_Parser *parser)
{
	return parser->opstack[parser->opstacksize-1];
}

static void pushopstack(poly_Parser *parser, const poly_Operator *op)
{
	if (parser->opstacksize > POLY_MAX_OP_STACK)
		throwerr(parser, "operator stack overflow");
	
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Pushing%s operator 0x%02X...\n", (op->unary ? " unary" : ""), op->type)
#endif
	
	parser->opstack[parser->opstacksize++] = op;
}

static const poly_Operator *popopstack(poly_Parser *parser)
{
	if (parser->opstacksize == 0)
		throwerr(parser, "operator stack empty");

#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Popping%s operator 0x%02X...\n",
		parser->opstack[parser->opstacksize-1]->unary ? " unary" : "",
		parser->opstack[parser->opstacksize-1]->type)
#endif

	return parser->opstack[--parser->opstacksize];
}

static void pushopcode(poly_VM *vm, const poly_Operator *op)
{
	if (!op->unary)
		switch (op->type)
		{
		case POLY_TOKEN_PLUS:
			mkcode(vm, POLY_INST_BIN_ADD, NULL); break;
		case POLY_TOKEN_MINUS:
			mkcode(vm, POLY_INST_BIN_SUB, NULL); break;
		case POLY_TOKEN_ASTERISK:
			mkcode(vm, POLY_INST_BIN_MUL, NULL); break;
		case POLY_TOKEN_SLASH:
			mkcode(vm, POLY_INST_BIN_DIV, NULL); break;
		case POLY_TOKEN_PRCNTSGN:
			mkcode(vm, POLY_INST_BIN_MOD, NULL); break;
		case POLY_TOKEN_CARET:
			mkcode(vm, POLY_INST_BIN_EXP, NULL); break;
		case POLY_TOKEN_EQEQ:
			mkcode(vm, POLY_INST_BIN_EQEQ, NULL); break;
		case POLY_TOKEN_UNEQ:
			mkcode(vm, POLY_INST_BIN_UNEQ, NULL); break;
		case POLY_TOKEN_LTEQ:
			mkcode(vm, POLY_INST_BIN_LTEQ, NULL); break;
		case POLY_TOKEN_GTEQ:
			mkcode(vm, POLY_INST_BIN_GTEQ, NULL); break;
		case POLY_TOKEN_AND:
			mkcode(vm, POLY_INST_BIN_AND, NULL); break;
		case POLY_TOKEN_OR:
			mkcode(vm, POLY_INST_BIN_OR, NULL); break;
		default:
			break;
		}
	else
		switch (op->type)
		{
		case POLY_TOKEN_MINUS:
			mkcode(vm, POLY_INST_UN_NEG, NULL); break;
		case POLY_TOKEN_NOT:
			mkcode(vm, POLY_INST_UN_NOT, NULL); break;
		default:
			break;
		}
}

/*
	value ::= BOOLEAN | NUMBER | IDENTIFIER | STRING

	arith-exp ::= '(' arith-exp ')'                           |
   				      arith-exp ( '^'           arith-exp ) * |
				      arith-exp ( ( '*' | '/' ) arith-exp ) * |
			          arith-exp ( ( '+' | '-' ) arith-exp ) * |
				      value
	
    logic-exp ::= exp       ( ( '==' | '!=' | '>=' | '<=' ) exp       ) * |
                  logic-exp ( ( 'and' | 'or' )              logic-exp ) * |

	exp ::= '-' arith-exp |
	            arith-exp |
			  '(' logic-exp ')' |
			'not' logic-exp     |
			      logic-exp
*/

static _Bool value(poly_VM *vm)
{
	if (islit(curtoken(&vm->lexer)->type))
	{
#ifdef POLY_DEBUG
		POLY_LOG_START(PRS)
		POLY_LOG("Got ")

		if (curtoken(&vm->lexer)->val->type == POLY_VAL_NUM)
			POLY_LOG("number: %.02f", curtoken(&vm->lexer)->val->num)
		else if (curtoken(&vm->lexer)->val->type == POLY_VAL_BOOL)
			POLY_LOG("boolean: %s", (curtoken(&vm->lexer)->val->bool ? "true" : "false"))
		else
			POLY_LOG("identifier: '%s'", curtoken(&vm->lexer)->val->str)
			
		POLY_LOG("\n")
		POLY_LOG_END
#endif
		
		mkcode(vm, POLY_INST_LITERAL, curtoken(&vm->lexer)->val);
		advtoken(&vm->lexer);
		return 1;
	}

	return 0;
}

static poly_Operator operators[] = {
	{ POLY_TOKEN_OPENRNDBRCKT,  0, POLY_OP_ASSOC_NONE,  0},
	{ POLY_TOKEN_CLOSERNDBRCKT, 0, POLY_OP_ASSOC_NONE,  0},
	{ POLY_TOKEN_CARET,         7, POLY_OP_ASSOC_RIGHT, 0},
	{ POLY_TOKEN_ASTERISK,      6, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_SLASH,         6, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_PRCNTSGN,      6, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_PLUS,          5, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_MINUS,         5, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_EQEQ, 			4, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_UNEQ,			4, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_LTEQ,			4, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_GTEQ,			4, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_NOT,           3, POLY_OP_ASSOC_NONE,  1},
	{ POLY_TOKEN_AND,           2, POLY_OP_ASSOC_LEFT,  0},
	{ POLY_TOKEN_OR,            1, POLY_OP_ASSOC_LEFT,  0},
	// Unary minus/plus must be at the bottom so it can be checked whether
	// the minus/plus is binary or unary at parsing phase
	{ POLY_TOKEN_MINUS,         8, POLY_OP_ASSOC_NONE,  1},
	{ POLY_TOKEN_PLUS,          8, POLY_OP_ASSOC_NONE,  1}
};

static const poly_Operator *operator(poly_VM *vm)
{
	poly_TokenType type = curtoken(&vm->lexer)->type;

	for (unsigned int i = 0; i < sizeof operators / sizeof operators[0]; i++)
	{
		if (operators[i].type == type)
		{
#ifdef POLY_DEBUG
			POLY_IMM_LOG(PRS, "Got operator 0x%02X\n", type)
#endif
			advtoken(&vm->lexer);
			return operators + i;
		}
	}
	
	return NULL;
}

static _Bool expression(poly_VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading expression...\n")
#endif

	const poly_Value *prevval = NULL;
	const poly_Value *val = NULL;

	poly_TokenType prevop = POLY_TOKEN_NONE;
	const poly_Operator *op = NULL;
	const poly_Operator *pop;

	while (1)
	{
		if (value(vm))
		{
			if (prevval != NULL && prevop != POLY_TOKEN_NONE)
			{
				val = prevtoken(&vm->lexer)->val; // ...is the parsed value

				if (isarithop(prevop))
					if (prevval->type != POLY_VAL_NUM || val->type != POLY_VAL_NUM)
						throwerr(&vm->parser, "the operands are illegal");
				
				if (isrelationop(prevop))
					if (prevop == POLY_TOKEN_GTEQ ||
					    prevop == POLY_TOKEN_LTEQ ||
						prevop == POLY_TOKEN_GT ||
						prevop == POLY_TOKEN_LT)
						if (prevval->type != POLY_VAL_NUM || val->type != POLY_VAL_NUM)
							throwerr(&vm->parser, "the operands are illegal");
			}
			
			prevval = prevtoken(&vm->lexer)->val; // ...is the data
			prevop = prevtoken(&vm->lexer)->type; // ...is literal type
			// ... so we got a value; continue.
			continue;
		}
		else if ((op = operator(vm)) == NULL)
			// ... but we didn't get any operator; break.
			break;
		
		switch (op->type)
		{
		case POLY_TOKEN_OPENRNDBRCKT:
		case POLY_TOKEN_CLOSERNDBRCKT:
		{
			if (op->type == POLY_TOKEN_OPENRNDBRCKT)
			{
				pushopstack(&vm->parser, op);
			}
			else if (op->type == POLY_TOKEN_CLOSERNDBRCKT)
			{
				while (gettopopstack(&vm->parser)->type != POLY_TOKEN_OPENRNDBRCKT)
				{
					pop = popopstack(&vm->parser);
					pushopcode(vm, pop);
				}

				if ((pop = popopstack(&vm->parser)) == NULL ||
					pop->type != POLY_TOKEN_OPENRNDBRCKT)
					throwerr(&vm->parser, "no matching \'(\'");
			}

			break;
		}
		default:
		{
			// Logic unary isn't here because it doesn't use the same token as
			// its respective binary operator.
			// Minus (-) and plus (+) have their binary and unary components.
			if (op->type == POLY_TOKEN_MINUS ||
			    op->type == POLY_TOKEN_PLUS)
				// Is unary if there is no previous token or
				//             previous token is any operator other than closing bracket.
				// Is binary otherwise.
				if (prevop == POLY_TOKEN_NONE ||
				    (isop(prevop) &&
					 prevop != POLY_TOKEN_CLOSERNDBRCKT))
				{
#ifdef POLY_DEBUG
					POLY_IMM_LOG(PRS, "... that is an unary operator (0x%02X)\n", prevop)
#endif

					if (op->type == POLY_TOKEN_PLUS)
						// Unary plus is useless; continue...
						continue; // while
					else
					{
						// Let's switch this `op` to the negation operator
						for (unsigned int i = 0; i < sizeof operators / sizeof operators[0]; i++)
						{
							if (operators[i].type == op->type && operators[i].unary)
							{
								op = &operators[i];
								break; // for
							}
						}
					}
				}

			while (vm->parser.opstacksize > 0 &&
				   ((op->assoc == POLY_OP_ASSOC_RIGHT && gettopopstack(&vm->parser)->prec > op->prec) ||
				    (op->assoc == POLY_OP_ASSOC_LEFT && gettopopstack(&vm->parser)->prec >= op->prec)))
			{
				pop = popopstack(&vm->parser);
				pushopcode(vm, pop);
			}

			pushopstack(&vm->parser, op);
		}
		}
		
		prevop = op->type;
	}

	while (vm->parser.opstacksize > 0 && (op = popopstack(&vm->parser)) != NULL)
	{
		if (op->type == POLY_TOKEN_OPENRNDBRCKT)
			throwerr(&vm->parser, "no matching \')\'");
		
		pushopcode(vm, op);
	}
	
	return 1;
}

static _Bool expressionlist(poly_VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Reading expression list...\n")
#endif

	if (expression(vm))
	{
		while (curtokenadv(&vm->lexer, POLY_TOKEN_COMMA))
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
	if (curtoken(&vm->lexer)->type == POLY_TOKEN_IDENTIFIER)
	{
#ifdef POLY_DEBUG
		POLY_IMM_LOG(PRS, "Got '%s' variable\n", curtoken(&vm->lexer)->val->str)
#endif
		mkcode(vm, POLY_INST_LITERAL, curtoken(&vm->lexer)->val);
		advtoken(&vm->lexer);

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
		while (curtokenadv(&vm->lexer, POLY_TOKEN_COMMA))
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
	    curtokenadv(&vm->lexer, POLY_TOKEN_EQ) &&
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

	while (curtoken(&vm->lexer)->type != POLY_TOKEN_EOF)
	{
		switch (curtoken(&vm->lexer)->type)
		{
		case POLY_TOKEN_NEWLINE:
			advtoken(&vm->lexer);
			vm->parser.curln++;
			break;
		case POLY_TOKEN_INDENT:
			advtoken(&vm->lexer);
			break;
		default:
			if (!statement(vm))
				throwerr(&vm->parser, "incorrect syntax");

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