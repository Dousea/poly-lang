#ifndef COMPILER_H_
#define COMPILER_H_

#include "value.h"

typedef enum
{
	TOKEN_OPENRNDBRCKT,
	TOKEN_CLOSERNDBRCKT,
	TOKEN_OPENCRLYBRCKT,
	TOKEN_CLOSECRLYBRCKT,
	TOKEN_OPENSQRBRCKT,
	TOKEN_CLOSESQRBRCKT,
	TOKEN_EQ,
	TOKEN_LT,
	TOKEN_GT,
	TOKEN_UNEQ,
	TOKEN_EQEQ,
	TOKEN_LTEQ,
	TOKEN_GTEQ,
	TOKEN_NMBRSGN,
	TOKEN_ATSGN,
	TOKEN_PRCNTSGN,
	TOKEN_QSTNMRK,
	TOKEN_EXCLMTNMRK,
	TOKEN_SINGLEQTMRK,
	TOKEN_DOUBLEQTMRK,
	TOKEN_CLN,
	TOKEN_CLNCLN,
	TOKEN_DOT,
	TOKEN_DOTDOT,
	TOKEN_DOTDOTDOT,
	TOKEN_COMMA,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_ASTERISK,
	TOKEN_SLASH,
	TOKEN_CARET,
	TOKEN_BACKSLASH,

	TOKEN_AND,
	TOKEN_CASE,
	TOKEN_CLASS,
	TOKEN_CONSTRUCTOR,
	TOKEN_CONTINUE,
	TOKEN_ELSE,
	TOKEN_END,
	TOKEN_EXTEND,
	TOKEN_EXTERN,
	TOKEN_FALSE,
	TOKEN_FUNCTION,
	TOKEN_GET,
	TOKEN_IF,
	TOKEN_INTERN,
	TOKEN_NAMESPACE,
	TOKEN_NOT,
	TOKEN_NULL,
	TOKEN_OR,
	TOKEN_PROPERTY,
	TOKEN_RETURN,
	TOKEN_SET,
	TOKEN_SUPER,
	TOKEN_TRUE,
	TOKEN_WHEN,
	TOKEN_WHILE,

	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,

	TOKEN_NEWLINE,
	TOKEN_SPACE,
	TOKEN_TAB,

	TOKEN_UNKNOWN,
	TOKEN_EOF
} TokenType;

typedef struct
{
	// Type of token
	TokenType type;
	// Character that points to [Lexer.source] of token's starting position
	const char *start;
	// Length of token from [start]
	int len;
	// Literal value
	Value value;
} Token;

typedef struct
{
	const char* word;
	TokenType type;
	int len;
} Keyword;

typedef struct
{
	// Source that's being lexed
	const char *source;
	// Current lexed character that points to [source]
	const char *curchar;
	// Character that points to [source] of the new token's start
	const char *tokenstart;
	// Lexical token stream that's used by parser
	Token *stream;
	// Total allocated tokens in [stream]
	int totaltoken;
} Lexer;

typedef struct
{
	Lexer lexer;
	// Current token that's being consumed
	Token *curtoken;
	// Current line position of token that's being consumed
	int curln;
	// Previous consumed token
	Token *prevtoken;
	// Indentation that will be used for scopes
	Token *indent;
} Parser;

typedef struct
{
	Parser parser;
} Compiler;

Compiler *Compiler_new(void);
void Compiler_compile(Compiler *compiler, const char *source);
void Compiler_free(Compiler *compiler);

#endif /* COMPILER_H_ */
