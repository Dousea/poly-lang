#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "poly.h"
#include "poly_value.h"
#include "poly_lexer.h"

static PolyKeyword keywords[] = {
		{ "and",         TOKEN_AND,         3  },
		{ "case",        TOKEN_CASE,        4  },
		{ "class",       TOKEN_CLASS,       5  },
		{ "constructor", TOKEN_CONSTRUCTOR, 11 },
		{ "continue",    TOKEN_CONTINUE,    8  },
		{ "else",        TOKEN_ELSE,        4  },
		{ "end",         TOKEN_END,         3  },
		{ "extend",      TOKEN_EXTEND,      6  },
		{ "extern",      TOKEN_EXTERN,      6  },
		{ "false",       TOKEN_FALSE,       5  },
		{ "function",    TOKEN_FUNCTION,    8  },
		{ "get",         TOKEN_GET,         3  },
		{ "if",          TOKEN_IF,          2  },
		{ "intern",      TOKEN_INTERN,      6  },
		{ "namespace",   TOKEN_NAMESPACE,   9  },
		{ "not",         TOKEN_NOT,         3  },
		{ "null",        TOKEN_NULL,        4  },
		{ "or",          TOKEN_OR,          2  },
		{ "property",    TOKEN_PROPERTY,    8  },
		{ "return",      TOKEN_RETURN,      6  },
		{ "set",         TOKEN_SET,         3  },
		{ "super",       TOKEN_SUPER,       5  },
		{ "true",        TOKEN_TRUE,        4  },
		{ "when",        TOKEN_WHEN,        4  },
		{ "while",       TOKEN_WHILE,       5  }
};

// Gets current character that's being read
static char curchar(PolyLexer *lexer)
{
	return *lexer->curchar;
}

// Gets next character that will be read
static char nextchar(PolyLexer *lexer)
{
	return *(lexer->curchar + 1);
}

// Gets previous character that's already read
static char prevchar(PolyLexer *lexer)
{
	return *(lexer->curchar - 1);
}

// Advances to the next character
static void advchar(PolyLexer *lexer)
{
	lexer->curchar++;
}

// If next character is [c] advance to the character then return 1, otherwise
// return 0
static _Bool conchar(PolyLexer *lexer, char c)
{
	if (nextchar(lexer) == c)
	{
		advchar(lexer);
		return 1;
	}
	else
		return 0;
}

// Gets length from token's start position to current character
static int lenchar(PolyLexer *lexer)
{
	return (int)(lexer->curchar - lexer->tokenstart) + 1;
}

// Creates a new token then put it in stream
static void mktoken(PolyLexer *lexer, PolyTokenType type)
{
	// Makes a new token
	PolyToken token;
	token.type = type;
	token.start = lexer->curchar;
	token.len = lenchar(lexer);
	// Allocates a new memory each time a new token is being made
	lexer->stream = polyAllocate(lexer->stream, sizeof(PolyToken) * ++lexer->totaltoken);
	lexer->stream[lexer->totaltoken - 1] = token;

#ifdef POLY_DEBUG
	printf("Creating token %d\n", type);
#endif
}

// Creates [second] token if next character is [c], otherwise [first] token
static void mkdbltoken(PolyLexer *lexer, char c, PolyTokenType first, PolyTokenType second)
{
	return (conchar(lexer, c) ? mktoken(lexer, second) : mktoken(lexer, first));
}

// Reads/skips comment
// TODO: Reads multi-line comment (#: ... :#)
static void readcomment(PolyLexer *lexer)
{
	while (nextchar(lexer) != '\n' || nextchar(lexer) != '\0')
		advchar(lexer);
}

// Reads name
static void readname(PolyLexer *lexer)
{
	while (isalnum(nextchar(lexer)) || nextchar(lexer) == '_')
		advchar(lexer);

	PolyTokenType type = TOKEN_IDENTIFIER;

	// Check if the name is reserved word/keyword
	for (int i = 0; keywords[i].word != NULL; i++)
	{
		if (memcmp(lexer->tokenstart, keywords[i].word, keywords[i].len) == 0)
		{
			type = keywords[i].type; break;
		}
	}

	mktoken(lexer, type);
}

// Reads number
// TODO: Reads hex? (low priority)
static void readnumber(PolyLexer *lexer)
{
	while (isdigit(nextchar(lexer)))
		advchar(lexer);

	if (conchar(lexer, '.'))
		if (isdigit(nextchar(lexer)))
			while (isdigit(nextchar(lexer)))
				advchar(lexer);

	if (conchar(lexer, 'e') || conchar(lexer, 'E'))
	{
		conchar(lexer, '-');

		if (!isdigit(nextchar(lexer)))
			// TODO: Gives a lexer error
			fprintf(stderr, "Unterminated scientific notation!");

		while (isdigit(nextchar(lexer)))
			advchar(lexer);
	}

	errno = 0;
	double num = strtod(lexer->tokenstart, NULL);

	if (errno == ERANGE)
		// TODO: Gives a lexer error
		fprintf(stderr, "Number literal is too large!");

	mktoken(lexer, TOKEN_NUMBER);

	PolyToken *t = lexer->stream + (lexer->totaltoken - 1);
	t->value.type = VALUE_NUMBER;
	t->value.num = num;
}

// Creates a lexical token stream to be parsed
void polyLex(PolyLexer *lexer)
{
#ifdef POLY_DEBUG
	printf("Lexing \"%s\" with lexer:%lu\n", lexer->source, (unsigned long)lexer);
#endif

	char c;

	while ((c = curchar(lexer)) != '\0')
	{
		lexer->tokenstart = lexer->curchar;

		switch (c)
		{
		case '(':
			mktoken(lexer, TOKEN_OPENRNDBRCKT); break;
		case ')':
			mktoken(lexer, TOKEN_CLOSERNDBRCKT); break;
		case '{':
			mktoken(lexer, TOKEN_OPENCRLYBRCKT); break;
		case '}':
			mktoken(lexer, TOKEN_CLOSECRLYBRCKT); break;
		case '[':
			mktoken(lexer, TOKEN_OPENSQRBRCKT); break;
		case ']':
			mktoken(lexer, TOKEN_CLOSESQRBRCKT); break;
		case '=':
			mkdbltoken(lexer, c, TOKEN_EQ, TOKEN_EQEQ); break;
		case '<':
			mkdbltoken(lexer, '=', TOKEN_LT, TOKEN_LTEQ); break;
		case '>':
			mkdbltoken(lexer, '=', TOKEN_GT, TOKEN_GTEQ); break;
		case '@':
			mktoken(lexer, TOKEN_ATSGN); break;
		case '%':
			mktoken(lexer, TOKEN_PRCNTSGN); break;
		case '?':
			mktoken(lexer, TOKEN_QSTNMRK); break;
		case '!':
			mkdbltoken(lexer, '=', TOKEN_EXCLMTNMRK, TOKEN_UNEQ); break;
		case '\'': case '"': // TODO: Reads a string
			break;
		case ':':
			mkdbltoken(lexer, c, TOKEN_CLN, TOKEN_CLNCLN); break;
		case '.': // TODO: Maybe read number with no leading zero (e.g. .75)? (low priority)
			if (conchar(lexer, c))
				mkdbltoken(lexer, c, TOKEN_DOTDOT, TOKEN_DOTDOTDOT);
			else
				mktoken(lexer, TOKEN_DOT);

			break;
		case ',':
			mktoken(lexer, TOKEN_COMMA); break;
		case '+':
			mktoken(lexer, TOKEN_PLUS); break;
		case '-':
			mktoken(lexer, TOKEN_MINUS); break;
		case '*':
			mktoken(lexer, TOKEN_ASTERISK); break;
		case '/':
			mktoken(lexer, TOKEN_SLASH); break;
		case '^':
			mktoken(lexer, TOKEN_CARET); break;
		case '\\': // TODO: Support for escape sequences (e.g. unicode) (low priority)
			mktoken(lexer, TOKEN_BACKSLASH); break;
		case '\n':
			mktoken(lexer, TOKEN_NEWLINE); break;
		case ' ': case '\t':
			// Conserve memory by not creating whitespace tokens if previous token is not new line
			if (prevchar(lexer) == '\n')
			{
				while (nextchar(lexer) == c)
					advchar(lexer);

				mktoken(lexer, (c == ' ' ? TOKEN_SPACE : TOKEN_TAB));
			}

			break;
		default:
			if (c == '#')
			{
				readcomment(lexer); break;
			}
			else if (isdigit(c))
			{
				readnumber(lexer); break;
			}
			else if (isalpha(c) || c == '_')
			{
				readname(lexer); break;
			}

			mktoken(lexer, TOKEN_UNKNOWN);
		}

		advchar(lexer);
	}

	mktoken(lexer, TOKEN_EOF);
}
