#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "poly.h"
#include "poly_value.h"
#include "poly_parser.h"

  //////////////////////
 // Lexical Analysis //
//////////////////////
static struct
{
	const char* word;
	TokenType type;
	int len;
} keywords[] = {
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
static char curchar(Parser *parser)
{
	return *parser->curchar;
}

// Gets next character that will be read
static char nextchar(Parser *parser)
{
	return *(parser->curchar + 1);
}

// Gets previous character that's already read
static char prevchar(Parser *parser)
{
	return *(parser->curchar - 1);
}

// Advances to the next character
static void advchar(Parser *parser)
{
	parser->curchar++;
}

// If next character is [c] advance to the character then return 1, otherwise
// return 0
static _Bool conchar(Parser *parser, char c)
{
	if (nextchar(parser) == c)
	{
		advchar(parser);
		return 1;
	}
	else
		return 0;
}

// Gets length from token's start position to current character
static int lenchar(Parser *parser)
{
	return (int)(parser->curchar - parser->tokenstart) + 1;
}

// Creates a new token then put it in tokenstream
static void mktoken(Parser *parser, TokenType type)
{
	// Makes a new token
	Token token;
	token.type = type;
	token.start = parser->curchar;
	token.len = lenchar(parser);
	// Allocates a new memory each time a new token is being made
	parser->tokenstream = polyAllocate(parser->tokenstream, sizeof(Token) * ++parser->totaltoken);
	parser->tokenstream[parser->totaltoken - 1] = token;

#ifdef POLY_DEBUG
	printf("Token %d created\n", type);
#endif
}

// Creates [second] token if next character is [c], otherwise [first] token
static void mkdbltoken(Parser *parser, char c, TokenType first, TokenType second)
{
	return (conchar(parser, c) ? mktoken(parser, second) : mktoken(parser, first));
}

// Reads/skips comment
static void readcomment(Parser *parser)
{
	// Multi-line comment -> #:<comment>:#
	if (conchar(parser, ':'))
	{
		int nested = 0;

		while (nextchar(parser) != '\0')
		{
			if (conchar(parser, '#'))
			{
				if (conchar(parser, ':'))
					nested++;
			}
			else if (conchar(parser, ':'))
			{
				if (conchar(parser, '#'))
					nested--;
			}

			if (nested < 0)
				return;

			advchar(parser);
		}

		// If we go here then this must be an unterminated comment
		fprintf(stderr, "Unterminated comment");
	}
	// Single-line comment -> #<comment>
	else
		while (nextchar(parser) != '\n' && nextchar(parser) != '\0')
			advchar(parser);
}

// Reads name
static void readname(Parser *parser)
{
	while (isalnum(nextchar(parser)) || nextchar(parser) == '_')
		advchar(parser);

	TokenType type = TOKEN_IDENTIFIER;

	// Check if the name is reserved word/keyword
	for (int i = 0; keywords[i].word != NULL; i++)
	{
		if (memcmp(parser->tokenstart, keywords[i].word, keywords[i].len) == 0)
		{
			type = keywords[i].type; break;
		}
	}

	mktoken(parser, type);

	if (type == TOKEN_FALSE || type == TOKEN_TRUE)
	{
		Token *t = parser->tokenstream + (parser->totaltoken - 1);
		t->value.type = VALUE_BOOLEAN;
		t->value.bool = (type == TOKEN_FALSE ? 0 : 1);
	}
}

// Reads number
// TODO: Reads hex? (low priority)
static void readnumber(Parser *parser)
{
	while (isdigit(nextchar(parser)))
		advchar(parser);

	if (conchar(parser, '.'))
		if (isdigit(nextchar(parser)))
			while (isdigit(nextchar(parser)))
				advchar(parser);

	if (conchar(parser, 'e') || conchar(parser, 'E'))
	{
		conchar(parser, '-');

		if (!isdigit(nextchar(parser)))
			// TODO: Gives a parser error
			fprintf(stderr, "Unterminated scientific notation!");

		while (isdigit(nextchar(parser)))
			advchar(parser);
	}

	errno = 0;
	double num = strtod(parser->tokenstart, NULL);

	if (errno == ERANGE)
		// TODO: Gives a parser error
		fprintf(stderr, "Number literal is too large!");

	mktoken(parser, TOKEN_NUMBER);

	Token *t = parser->tokenstream + (parser->totaltoken - 1);
	t->value.type = VALUE_NUMBER;
	t->value.num = num;
}

// Creates a lexical token tokenstream to be parsed
static void lex(Parser *parser)
{
	// Is first indentation already occured?
	_Bool firstindent = 0;
	// Indentation that will be used for scopes
	struct {
		char c;
		int len;
	} indent;
	// Current character that's being lexed
	char c;

	while ((c = curchar(parser)) != '\0')
	{
		parser->tokenstart = parser->curchar;

		switch (c)
		{
		case '(':
			mktoken(parser, TOKEN_OPENRNDBRCKT); break;
		case ')':
			mktoken(parser, TOKEN_CLOSERNDBRCKT); break;
		case '{':
			mktoken(parser, TOKEN_OPENCRLYBRCKT); break;
		case '}':
			mktoken(parser, TOKEN_CLOSECRLYBRCKT); break;
		case '[':
			mktoken(parser, TOKEN_OPENSQRBRCKT); break;
		case ']':
			mktoken(parser, TOKEN_CLOSESQRBRCKT); break;
		case '=':
			mkdbltoken(parser, c, TOKEN_EQ, TOKEN_EQEQ); break;
		case '<':
			mkdbltoken(parser, '=', TOKEN_LT, TOKEN_LTEQ); break;
		case '>':
			mkdbltoken(parser, '=', TOKEN_GT, TOKEN_GTEQ); break;
		case '@':
			mktoken(parser, TOKEN_ATSGN); break;
		case '%':
			mktoken(parser, TOKEN_PRCNTSGN); break;
		case '?':
			mktoken(parser, TOKEN_QSTNMRK); break;
		case '!':
			mkdbltoken(parser, '=', TOKEN_EXCLMTNMRK, TOKEN_UNEQ); break;
		case '\'': case '"': // TODO: Reads a string
			break;
		case ':':
			mkdbltoken(parser, c, TOKEN_CLN, TOKEN_CLNCLN); break;
		case '.': // TODO: Maybe reads number with no leading zero (e.g. .75)? (low priority)
			if (conchar(parser, c))
				mkdbltoken(parser, c, TOKEN_DOTDOT, TOKEN_DOTDOTDOT);
			else
				mktoken(parser, TOKEN_DOT);

			break;
		case ',':
			mktoken(parser, TOKEN_COMMA); break;
		case '+':
			mktoken(parser, TOKEN_PLUS); break;
		case '-':
			mktoken(parser, TOKEN_MINUS); break;
		case '*':
			mktoken(parser, TOKEN_ASTERISK); break;
		case '/':
			mktoken(parser, TOKEN_SLASH); break;
		case '^':
			mktoken(parser, TOKEN_CARET); break;
		case '\\': // TODO: Support for escape sequences (e.g. unicode) (low priority)
			mktoken(parser, TOKEN_BACKSLASH); break;
		case '\n':
			mktoken(parser, TOKEN_NEWLINE); break;
		case ' ': case '\t':
			if (prevchar(parser) == '\n')
			{
				if (firstindent == 0)
				{
					firstindent = 1;
					indent.c = c;

					while (nextchar(parser) == c)
						advchar(parser);

					indent.len = lenchar(parser);

					mktoken(parser, TOKEN_INDENT);

					Token *t = parser->tokenstream + (parser->totaltoken - 1);
					t->len = 1;
				}
				else
				{
					if (c == indent.c)
					{
						while (nextchar(parser) == c)
							advchar(parser);

						int len = lenchar(parser);

						if (len % indent.len == 0)
						{
							mktoken(parser, TOKEN_INDENT);

							Token *t = parser->tokenstream + (parser->totaltoken - 1);
							t->len = len / indent.len;
						}
						else // TODO: Gives a parser error
							fprintf(stderr, "Please have a consistent type of indentation\n");
					}
					else // TODO: Gives a parser error
						fprintf(stderr, "Please have a consistent type of indentation\n");
				}
			}

			break;
		default:
			if (c == '#')
			{
				readcomment(parser); break;
			}
			else if (isdigit(c))
			{
				readnumber(parser); break;
			}
			else if (isalpha(c) || c == '_')
			{
				readname(parser); break;
			}

			mktoken(parser, TOKEN_UNKNOWN);
		}

		advchar(parser);
	}

	mktoken(parser, TOKEN_EOF);
}

  ////////////////////////
 // Syntactic Analysis //
////////////////////////
/*
static _Bool isexp(PolyParser *parser, PolyTokenType type)
{
	switch (type)
	{
	case TOKEN_NULL: case TOKEN_TRUE: case TOKEN_FALSE:
		return 1;
	default:
	}
}

static _Bool isbinop(PolyParser *parser, PolyTokenType type)
{
	switch (type)
	{
	case TOKEN_PLUS: case TOKEN_MINUS: case TOKEN_ASTERISK: case TOKEN_SLASH:
	case TOKEN_CARET: case TOKEN_PRCNTSGN:
	case TOKEN_UNEQ: case TOKEN_LT: case TOKEN_LTEQ: case TOKEN_EQEQ: case TOKEN_GTEQ: case TOKEN_GT:
	case TOKEN_AND: case TOKEN_OR:
		return 1;
	default:
		return 0;
	}
}

static _Bool isunop(PolyParser *parser, PolyTokenType type)
{
	switch (type)
	{
	case TOKEN_MINUS: case TOKEN_NOT:
		return 1;
	default:
		return 0;
	}
}
*/

// Checks if the lexical token stream is at an allowable form and creates bytecodes
void polyParse(Parser *parser, const char *source)
{
#ifdef POLY_DEBUG
	printf("Parsing %s with memory:%lu\n", source, (unsigned long)parser);
#endif

	parser->source = source;
	parser->curchar = parser->source;

#ifdef POLY_DEBUG
	printf("Creating tokens..\n");
#endif

	lex(parser);

#ifdef POLY_DEBUG
	printf("Parsing tokens..\n");
#endif

	// Current token that's being consumed
	Token *curtoken = parser->tokenstream;
	// Previous token that already consumed
	Token *prevtoken = curtoken;
	// Current line position of token that's being consumed
	int curln = 1;
	// Current scope level
	int curscopelvl = 0;
	// Current scope
	Scope *curscope = *(parser->scope + curscopelvl);

	while (curtoken->type != TOKEN_EOF)
	{
		printf("Reading token %d with value of %f\n",
				curtoken->type,
				(curtoken->value.type == VALUE_NUMBER ? curtoken->value.num : (double)curtoken->value.bool));

		switch (curtoken->type)
		{
		case TOKEN_NEWLINE:
			curln++; break;
		// Previous token is new line and current token is whitespace? Must be indentation!
		case TOKEN_INDENT:
			; // Without this semicolon, GCC compiler would show an error
			int newscopelvl;
			newscopelvl = curtoken->len;

			// Difference from newscope to curscope must be 1
			if (abs(newscopelvl - curscopelvl) != 1)
				// TODO: Gives a parser error
				fprintf(stderr, "Incorrect scoping from level %d to %d\n", curscopelvl, newscopelvl);

			if (newscopelvl > MAX_SCOPES)
				// TODO: Gives a parser error
				fprintf(stderr, "Exceeds allowed scopes");

			curscopelvl = newscopelvl;
			curscope = *(parser->scope + curscopelvl);

#ifdef POLY_DEBUG
			printf("Indentation level %d\n", curscopelvl);
#endif

			break;
		case TOKEN_UNKNOWN:
			// TODO: Gives a parser error
			fprintf(stderr, "An unknown token is found\n"); break;
		default:
			// If current token is not whitespace but previous token is new
			// line then reset current scope to 0 as no indentation
			// detected
			if (prevtoken->type == TOKEN_NEWLINE && curscopelvl != 0)
			{
				curscopelvl = 0;
				curscope = *(parser->scope + curscopelvl);

#ifdef POLY_DEBUG
				printf("Indentation level 0\n");
#endif
			}

			break;
		}

		prevtoken = curtoken++;
	}
}
