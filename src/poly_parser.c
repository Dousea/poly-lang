#include <stdio.h>

#include "poly.h"
#include "poly_parser.h"

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

// Checks if the lexical token stream is at an allowable form and creates a syntax tree
void polyParse(PolyParser *parser)
{
#ifdef POLY_DEBUG
	printf("Parsing tokens with parser:%lu\n", (unsigned long)parser);
#endif

	PolyLexer *lexer = &parser->lexer;
	// Current token that's being consumed
	PolyToken *curtoken = lexer->stream;
	// Previous token that already consumed
	PolyToken *prevtoken = curtoken;
	// Current line position of token that's being consumed
	int curln = 1;
	// Indentation that will be used for scopes
	PolyToken indent;
	// Current scope (?)
	int curscope = 0;

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
		case TOKEN_SPACE: case TOKEN_TAB:
			if (prevtoken->type == TOKEN_NEWLINE)
			{
				// There's no [parser]->indent yet so we must create it for reading further indents
				// So we can throw errors at programmers if they don't obey their own indent!
				if (indent.type != TOKEN_SPACE && indent.type != TOKEN_TAB)
				{
					printf("First indentation detected - indentation level 0\n");

					indent.type = curtoken->type;
					indent.len = curtoken->len;

					// TODO: Scoping
				}
				else
				{
					if (curtoken->type == indent.type)
					{
						if (curtoken->len % indent.len == 0)
						{
							// TODO: Scoping
							curscope = curtoken->len / indent.len;

							printf("Indentation level %d", curscope);
						}
						else // TODO: Gives a parser error
							fprintf(stderr, "Please have a consistent type of indentation\n");
					}
					else // TODO: Gives a parser error
						fprintf(stderr, "Please have a consistent type of indentation\n");
				}
			}

			break;
		case TOKEN_UNKNOWN:
			// TODO: Gives a parser error
			fprintf(stderr, "An unknown token is found\n"); break;
		default:
			break;
		}

		prevtoken = curtoken++;
	}
}
