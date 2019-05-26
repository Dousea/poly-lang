#ifndef POLY_LEX_H
#define POLY_LEX_H

#include "poly_value.h"

typedef enum poly_TokenType
{
	POLY_TOKEN_NONE,
	
	POLY_TOKEN_OPENRNDBRCKT,
	POLY_TOKEN_CLOSERNDBRCKT,
	POLY_TOKEN_OPENCRLYBRCKT,
	POLY_TOKEN_CLOSECRLYBRCKT,
	POLY_TOKEN_OPENSQRBRCKT,
	POLY_TOKEN_CLOSESQRBRCKT,
	POLY_TOKEN_EQ,
	POLY_TOKEN_LT,
	POLY_TOKEN_GT,
	POLY_TOKEN_UNEQ,
	POLY_TOKEN_EQEQ,
	POLY_TOKEN_LTEQ,
	POLY_TOKEN_GTEQ,
	POLY_TOKEN_NMBRSGN,
	POLY_TOKEN_ATSGN,
	POLY_TOKEN_PRCNTSGN,
	POLY_TOKEN_QSTNMRK,
	POLY_TOKEN_EXCLMTNMRK,
	POLY_TOKEN_SINGLEQTMRK,
	POLY_TOKEN_DOUBLEQTMRK,
	POLY_TOKEN_CLN,
	POLY_TOKEN_CLNCLN,
	POLY_TOKEN_DOT,
	POLY_TOKEN_DOTDOT,
	POLY_TOKEN_DOTDOTDOT,
	POLY_TOKEN_COMMA,
	POLY_TOKEN_PLUS,
	POLY_TOKEN_MINUS,
	POLY_TOKEN_ASTERISK,
	POLY_TOKEN_SLASH,
	POLY_TOKEN_CARET,
	POLY_TOKEN_BACKSLASH,

	POLY_TOKEN_AND,
	POLY_TOKEN_BREAK,
	POLY_TOKEN_CONTINUE,
	POLY_TOKEN_DO,
	POLY_TOKEN_ELSE,
	POLY_TOKEN_END,
	POLY_TOKEN_FALSE,
	POLY_TOKEN_FOR,
	POLY_TOKEN_FUNCTION,
	POLY_TOKEN_IF,
	POLY_TOKEN_NOT,
	POLY_TOKEN_NULL,
	POLY_TOKEN_OR,
	POLY_TOKEN_REPEAT,
	POLY_TOKEN_RETURN,
	POLY_TOKEN_TRUE,
	POLY_TOKEN_UNTIL,
	POLY_TOKEN_WHILE,

	POLY_TOKEN_IDENTIFIER,
	POLY_TOKEN_NUMBER,

	POLY_TOKEN_NEWLINE,
	POLY_TOKEN_INDENT,

	POLY_TOKEN_EOF
} poly_TokenType;

typedef struct poly_Token
{
	poly_TokenType type;
	const char *start;
	unsigned char len;
	poly_Value *val;
} poly_Token;

typedef struct poly_TokenStream
{
	poly_Token *stream;
	const poly_Token *cur;
	size_t size;
	size_t allotedmem;
	size_t maxmem;
} poly_TokenStream;

typedef struct poly_Lexer
{
	const char *src;
	const char *curchar;
	const char *tokenstart;
	poly_TokenStream tokenstream;
	size_t curln;
} poly_Lexer;

#endif
