#ifndef POLY_PARSE_H_
#define POLY_PARSE_H_

// Maximum operators inside an expression
#define POLY_MAX_OP_STACK   64

typedef enum poly_OperatorAssociativity
{
	POLY_OP_ASSOC_NONE,
	POLY_OP_ASSOC_LEFT,
	POLY_OP_ASSOC_RIGHT
} poly_OperatorAssociativity;

typedef struct poly_Operator
{
	poly_TokenType type;
	int prec;
	poly_OperatorAssociativity assoc;
	_Bool unary;
} poly_Operator;

typedef struct poly_Parser
{
	const poly_Operator *opstack[POLY_MAX_OP_STACK];
	size_t opstacksize;
	size_t curln;
} poly_Parser;

#endif
