#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "poly_config.h"
#include "poly_vm.h"
#include "poly_log.h"

static void throwerr(poly_Lexer *lexer, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "\x1B[1;31mLexing error: line %zu: ", lexer->curln);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\x1B[0m\n");
	va_end(args);
	exit(EXIT_FAILURE);
}

static struct poly_Keyword
{
	const char *word;
	poly_TokenType type;
	int len;
} keyword[] = {
		{ "and",         POLY_TOKEN_AND,         3  },
		{ "break",       POLY_TOKEN_BREAK,       5  },
		{ "continue",    POLY_TOKEN_CONTINUE,    8  },
		{ "do",          POLY_TOKEN_DO,          2  },
		{ "else",        POLY_TOKEN_ELSE,        4  },
		{ "false",       POLY_TOKEN_FALSE,       5  },
		{ "for",         POLY_TOKEN_FOR,         3  },
		{ "function",    POLY_TOKEN_FUNCTION,    8  },
		{ "if",          POLY_TOKEN_IF,          2  },
		{ "not",         POLY_TOKEN_NOT,         3  },
		{ "null",        POLY_TOKEN_NULL,        4  },
		{ "or",          POLY_TOKEN_OR,          2  },
		{ "repeat",      POLY_TOKEN_REPEAT,      6  },
		{ "return",      POLY_TOKEN_RETURN,      6  },
		{ "true",        POLY_TOKEN_TRUE,        4  },
		{ "until",       POLY_TOKEN_UNTIL,       5  },
		{ "while",       POLY_TOKEN_WHILE,       5  }
};

// Gets current character that's being read
static char curchar(poly_Lexer *lexer)
{
	return *lexer->curchar;
}

// Gets next character that will be read
static char nextchar(poly_Lexer *lexer)
{
	return *(lexer->curchar + 1);
}

// Gets previous character that's already read
static char prevchar(poly_Lexer *lexer)
{
	return *(lexer->curchar - 1);
}

// Advances to the next character
static void advchar(poly_Lexer *lexer)
{
	lexer->curchar++;
}

// If next character is [c] advance to the character then return 1, otherwise
// return 0
static _Bool nextcharadv(poly_Lexer *lexer, char c)
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
static size_t lenchar(poly_Lexer *lexer)
{
	return (size_t)(lexer->curchar - lexer->tokenstart) + 1;
}

// Creates a new token then put it in tokenstream
static poly_Token *mktoken(poly_VM *vm, poly_TokenType type)
{
	// Makes a new token
	poly_Token token;
	token.type = type;
	token.start = vm->lexer.tokenstart;
	
	size_t len = lenchar(&vm->lexer);

#define MAX_CHARS CHAR_BIT * sizeof(token.len)
	if (len > MAX_CHARS)
		throwerr(&vm->lexer, "length of characters is too big! (more than %zu)", MAX_CHARS);
#undef MAX_CHARS
	
	token.len = len;

	// Keep track on our memory allocation here
	size_t size = sizeof(poly_Token);

	if ((vm->lexer.tokenstream.allotedmem + size) > vm->lexer.tokenstream.maxmem)
	{
		vm->lexer.tokenstream.maxmem = POLY_ALLOC_MEM(vm->lexer.tokenstream.maxmem);
		vm->lexer.tokenstream.stream = vm->config->alloc(vm->lexer.tokenstream.stream,
		                                                  vm->lexer.tokenstream.maxmem);

#ifdef POLY_DEBUG
		POLY_IMM_LOG(MEM, "Resized token stream memory to %zu bytes\n", vm->lexer.tokenstream.maxmem)
#endif
	}

	vm->lexer.tokenstream.allotedmem += size;
	vm->lexer.tokenstream.stream[++vm->lexer.tokenstream.size - 1] = token;

#ifdef POLY_DEBUG
	POLY_IMM_LOG(MEM, "0x%lX: created token 0x%02X\n",
		(unsigned long)(vm->lexer.tokenstream.stream + (vm->lexer.tokenstream.size - 1)),
		type)
#endif

	return (vm->lexer.tokenstream.stream + (vm->lexer.tokenstream.size - 1));
}

// Creates [second] token if next character is [c], otherwise [first] token
static poly_Token *mkterntoken(poly_VM *vm, char c, poly_TokenType first, poly_TokenType second)
{
	return (nextcharadv(&vm->lexer, c) ? mktoken(vm, second) : mktoken(vm, first));
}

// Creates a lexical token stream to be parsed
POLY_LOCAL void lex(poly_VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(LEX, "Tokenizating...\n")
#endif

	// Is first indentation already occured?
	_Bool firstindent = 0;
	// Indentation that will be used for scopes
	struct {
		char c;
		int len;
	} indent;
	// Current character that's being lexed
	char c;

	vm->lexer.curln = 0;

	while ((c = curchar(&vm->lexer)) != '\0')
	{
		vm->lexer.tokenstart = vm->lexer.curchar;

		switch (c)
		{
		case '(':
			mktoken(vm, POLY_TOKEN_OPENRNDBRCKT); break;
		case ')':
			mktoken(vm, POLY_TOKEN_CLOSERNDBRCKT); break;
		case '{':
			mktoken(vm, POLY_TOKEN_OPENCRLYBRCKT); break;
		case '}':
			mktoken(vm, POLY_TOKEN_CLOSECRLYBRCKT); break;
		case '[':
			mktoken(vm, POLY_TOKEN_OPENSQRBRCKT); break;
		case ']':
			mktoken(vm, POLY_TOKEN_CLOSESQRBRCKT); break;
		case '=':
			mkterntoken(vm, c, POLY_TOKEN_EQ, POLY_TOKEN_EQEQ); break;
		case '<':
			mkterntoken(vm, '=', POLY_TOKEN_LT, POLY_TOKEN_LTEQ); break;
		case '>':
			mkterntoken(vm, '=', POLY_TOKEN_GT, POLY_TOKEN_GTEQ); break;
		case '@':
			mktoken(vm, POLY_TOKEN_ATSGN); break;
		case '%':
			mktoken(vm, POLY_TOKEN_PRCNTSGN); break;
		case '?':
			mktoken(vm, POLY_TOKEN_QSTNMRK); break;
		case '!':
			mkterntoken(vm, '=', POLY_TOKEN_EXCLMTNMRK, POLY_TOKEN_UNEQ); break;
		case '\'': case '"': // TODO: Reads a string
			break;
		case ':':
			mkterntoken(vm, c, POLY_TOKEN_CLN, POLY_TOKEN_CLNCLN); break;
		case '.': // TODO: Maybe reads number with no leading zero (e.g. .75)? (low priority)
			if (nextcharadv(&vm->lexer, c))
				mkterntoken(vm, c, POLY_TOKEN_DOTDOT, POLY_TOKEN_DOTDOTDOT);
			else
				mktoken(vm, POLY_TOKEN_DOT);

			break;
		case ',':
			mktoken(vm, POLY_TOKEN_COMMA); break;
		case '+':
			mktoken(vm, POLY_TOKEN_PLUS); break;
		case '-':
			mktoken(vm, POLY_TOKEN_MINUS); break;
		case '*':
			mktoken(vm, POLY_TOKEN_ASTERISK); break;
		case '/':
			mktoken(vm, POLY_TOKEN_SLASH); break;
		case '^':
			mktoken(vm, POLY_TOKEN_CARET); break;
		case '\\': // TODO: Support for escape sequences (e.g. unicode) (low priority)
			mktoken(vm, POLY_TOKEN_BACKSLASH); break;
		case '\n':
			vm->lexer.curln++;
			mktoken(vm, POLY_TOKEN_NEWLINE);
			break;
		case ' ': case '\t':
			if (prevchar(&vm->lexer) == '\n')
			{
				if (firstindent == 0)
				{
					firstindent = 1;
					indent.c = c;

					while (nextchar(&vm->lexer) == c)
						advchar(&vm->lexer);

					indent.len = lenchar(&vm->lexer);

					poly_Token *t = mktoken(vm, POLY_TOKEN_INDENT);
					t->len = 1;
				}
				else
				{
					if (c == indent.c)
					{
						while (nextchar(&vm->lexer) == c)
							advchar(&vm->lexer);

						int len = lenchar(&vm->lexer);

						if (len % indent.len == 0)
						{
							poly_Token *t = mktoken(vm, POLY_TOKEN_INDENT);
							t->len = len / indent.len;
						}
						else
							throwerr(&vm->lexer, "inconsistent type of identation");
					}
					else
						throwerr(&vm->lexer, "inconsistent type of indentation");
				}
			}

			break;
		case '#':
			// Multi-line comment -> #:<comment>:#
			if (nextcharadv(&vm->lexer, ':'))
			{
				int nested = 0;

				while (nextchar(&vm->lexer) != '\0')
				{
					if (nextcharadv(&vm->lexer, '#'))
					{
						if (nextcharadv(&vm->lexer, ':'))
							nested++;
					}
					else if (nextcharadv(&vm->lexer, ':'))
					{
						if (nextcharadv(&vm->lexer, '#'))
							nested--;
					}

					if (nested < 0)
						break;

					advchar(&vm->lexer);
				}

				// If we go here then this must be an unterminated comment
				throwerr(&vm->lexer, "unterminated comment");
			}
			// Single-line comment -> #<comment>
			else
				while (nextchar(&vm->lexer) != '\n' && nextchar(&vm->lexer) != '\0')
					advchar(&vm->lexer);

			break;
		default:
			if (isdigit(c)) // Reads number!
				// TODO: Reads hex? (low priority)
			{
				while (isdigit(nextchar(&vm->lexer)))
					advchar(&vm->lexer);

				if (nextcharadv(&vm->lexer, '.'))
					if (isdigit(nextchar(&vm->lexer)))
						while (isdigit(nextchar(&vm->lexer)))
							advchar(&vm->lexer);

				if (nextcharadv(&vm->lexer, 'e') || nextcharadv(&vm->lexer, 'E'))
				{
					nextcharadv(&vm->lexer, '-');

					if (!isdigit(nextchar(&vm->lexer)))
						throwerr(&vm->lexer, "unterminated scientific notation");

					while (isdigit(nextchar(&vm->lexer)))
						advchar(&vm->lexer);
				}

				errno = 0;
				double num = strtod(vm->lexer.tokenstart, NULL);

				if (errno == ERANGE)
					throwerr(&vm->lexer, "number literal is too large");

				poly_Token *t = mktoken(vm, POLY_TOKEN_NUMBER);
				poly_Value *val = (poly_Value*)vm->config->alloc(NULL, sizeof(poly_Value));
				val->type = POLY_VAL_NUM;
				val->num = num;
				t->val = val;

				break;
			}
			else if (isalpha(c) || c == '_') // Reads name!
			{
				while (isalnum(nextchar(&vm->lexer)) || nextchar(&vm->lexer) == '_')
					advchar(&vm->lexer);

				poly_TokenType type = POLY_TOKEN_IDENTIFIER;

				// Check if the name is reserved word/keyword
				for (int i = 0; keyword[i].word != NULL; i++)
					if (memcmp(&vm->lexer.tokenstart, keyword[i].word, keyword[i].len) == 0)
					{
						type = keyword[i].type;
						break;
					}

				poly_Token *t = mktoken(vm, type);
				poly_Value *val = (poly_Value*)vm->config->alloc(NULL, sizeof(poly_Value));

				if (type == POLY_TOKEN_FALSE || type == POLY_TOKEN_TRUE)
				{
					val->type = POLY_VAL_BOOL;
					val->bool = (type == POLY_TOKEN_FALSE ? 0 : 1);
				}
				else if (type == POLY_TOKEN_IDENTIFIER)
				{
					val->str = vm->config->alloc(NULL, CHAR_BIT * sizeof(t->len));
					strncpy(val->str, t->start, t->len);
					val->str[t->len] = '\0';
					val->type = POLY_VAL_ID;
				}

				t->val = val;

				break;
			}
			
			throwerr(&vm->lexer, "unknown symbol");
		}

		advchar(&vm->lexer);
	}

	mktoken(vm, POLY_TOKEN_EOF);


#ifdef POLY_DEBUG
	POLY_IMM_LOG(LEX, "Allocated %zu bytes for %zu tokens\n",
		vm->lexer.tokenstream.allotedmem,
		vm->lexer.tokenstream.size)
#endif
}