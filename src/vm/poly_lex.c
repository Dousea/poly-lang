#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "poly_config.h"
#include "poly_vm.h"
#include "poly_log.h"

static struct
{
	const char* word;
	TokenType type;
	int len;
} keywords[] = {
		{ "and",         TOKEN_AND,         3  },
		{ "break",       TOKEN_BREAK,       5  },
		{ "continue",    TOKEN_CONTINUE,    8  },
		{ "do",          TOKEN_DO,          2  },
		{ "else",        TOKEN_ELSE,        4  },
		{ "false",       TOKEN_FALSE,       5  },
		{ "for",         TOKEN_FOR,         3  },
		{ "function",    TOKEN_FUNCTION,    8  },
		{ "if",          TOKEN_IF,          2  },
		{ "not",         TOKEN_NOT,         3  },
		{ "null",        TOKEN_NULL,        4  },
		{ "or",          TOKEN_OR,          2  },
		{ "repeat",      TOKEN_REPEAT,      6  },
		{ "return",      TOKEN_RETURN,      6  },
		{ "true",        TOKEN_TRUE,        4  },
		{ "until",       TOKEN_UNTIL,       5  },
		{ "while",       TOKEN_WHILE,       5  }
};

// Gets current character that's being read
static char curchar(Lexer *lexer)
{
	return *lexer->curchar;
}

// Gets next character that will be read
static char nextchar(Lexer *lexer)
{
	return *(lexer->curchar + 1);
}

// Gets previous character that's already read
static char prevchar(Lexer *lexer)
{
	return *(lexer->curchar - 1);
}

// Advances to the next character
static void advchar(Lexer *lexer)
{
	lexer->curchar++;
}

// If next character is [c] advance to the character then return 1, otherwise
// return 0
static _Bool nextcharadv(Lexer *lexer, char c)
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
static int lenchar(Lexer *lexer)
{
	return (int)(lexer->curchar - lexer->tokenstart) + 1;
}

// Creates a new token then put it in tokenstream
static Token* mktoken(VM *vm, TokenType type)
{
	// Makes a new token
	Token token;
	token.type = type;
	token.start = vm->parser.lexer.curchar;
	token.len = lenchar(&vm->parser.lexer);

	// Keep track on our memory allocation here
	size_t size = sizeof(Token);

	if ((vm->parser.tokenstream.allocatedmemory + size) > vm->parser.tokenstream.maxmemory)
	{
		vm->parser.tokenstream.maxmemory = POLY_ALLOCATE_MEM(vm->parser.tokenstream.maxmemory);
		vm->parser.tokenstream.stream = vm->config->allocator(vm->parser.tokenstream.stream,
		                                               vm->parser.tokenstream.maxmemory);

#ifdef POLY_DEBUG
		POLY_IMM_LOG(LEX, "Resized token stream memory to %u bytes\n", vm->parser.tokenstream.maxmemory)
#endif
	}

	vm->parser.tokenstream.allocatedmemory += size;
	vm->parser.tokenstream.stream[++vm->parser.tokenstream.total - 1] = token;

#ifdef POLY_DEBUG
	POLY_IMM_LOG(LEX, "Created token 0x%02X\n", type)
#endif

	return (vm->parser.tokenstream.stream + (vm->parser.tokenstream.total - 1));
}

// Creates [second] token if next character is [c], otherwise [first] token
static Token* mkdbltoken(VM *vm, char c, TokenType first, TokenType second)
{
	return (nextcharadv(&vm->parser.lexer, c) ? mktoken(vm, second) : mktoken(vm, first));
}

// Creates a lexical token stream to be parsed
POLY_LOCAL void lex(VM *vm)
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

	while ((c = curchar(&vm->parser.lexer)) != '\0')
	{
		vm->parser.lexer.tokenstart = vm->parser.lexer.curchar;

		switch (c)
		{
		case '(':
			mktoken(vm, TOKEN_OPENRNDBRCKT); break;
		case ')':
			mktoken(vm, TOKEN_CLOSERNDBRCKT); break;
		case '{':
			mktoken(vm, TOKEN_OPENCRLYBRCKT); break;
		case '}':
			mktoken(vm, TOKEN_CLOSECRLYBRCKT); break;
		case '[':
			mktoken(vm, TOKEN_OPENSQRBRCKT); break;
		case ']':
			mktoken(vm, TOKEN_CLOSESQRBRCKT); break;
		case '=':
			mkdbltoken(vm, c, TOKEN_EQ, TOKEN_EQEQ); break;
		case '<':
			mkdbltoken(vm, '=', TOKEN_LT, TOKEN_LTEQ); break;
		case '>':
			mkdbltoken(vm, '=', TOKEN_GT, TOKEN_GTEQ); break;
		case '@':
			mktoken(vm, TOKEN_ATSGN); break;
		case '%':
			mktoken(vm, TOKEN_PRCNTSGN); break;
		case '?':
			mktoken(vm, TOKEN_QSTNMRK); break;
		case '!':
			mkdbltoken(vm, '=', TOKEN_EXCLMTNMRK, TOKEN_UNEQ); break;
		case '\'': case '"': // TODO: Reads a string
			break;
		case ':':
			mkdbltoken(vm, c, TOKEN_CLN, TOKEN_CLNCLN); break;
		case '.': // TODO: Maybe reads number with no leading zero (e.g. .75)? (low priority)
			if (nextcharadv(&vm->parser.lexer, c))
				mkdbltoken(vm, c, TOKEN_DOTDOT, TOKEN_DOTDOTDOT);
			else
				mktoken(vm, TOKEN_DOT);

			break;
		case ',':
			mktoken(vm, TOKEN_COMMA); break;
		case '+':
			mktoken(vm, TOKEN_PLUS); break;
		case '-':
			mktoken(vm, TOKEN_MINUS); break;
		case '*':
			mktoken(vm, TOKEN_ASTERISK); break;
		case '/':
			mktoken(vm, TOKEN_SLASH); break;
		case '^':
			mktoken(vm, TOKEN_CARET); break;
		case '\\': // TODO: Support for escape sequences (e.g. unicode) (low priority)
			mktoken(vm, TOKEN_BACKSLASH); break;
		case '\n':
			mktoken(vm, TOKEN_NEWLINE); break;
		case ' ': case '\t':
			if (prevchar(&vm->parser.lexer) == '\n')
			{
				if (firstindent == 0)
				{
					firstindent = 1;
					indent.c = c;

					while (nextchar(&vm->parser.lexer) == c)
						advchar(&vm->parser.lexer);

					indent.len = lenchar(&vm->parser.lexer);

					Token *t = mktoken(vm, TOKEN_INDENT);
					t->len = 1;
				}
				else
				{
					if (c == indent.c)
					{
						while (nextchar(&vm->parser.lexer) == c)
							advchar(&vm->parser.lexer);

						int len = lenchar(&vm->parser.lexer);

						if (len % indent.len == 0)
						{
							Token *t = mktoken(vm, TOKEN_INDENT);
							t->len = len / indent.len;
						}
						else // TODO: Gives a lexer error
							fprintf(stderr, "Please have a consistent type of indentation\n");
					}
					else // TODO: Gives a lexer error
						fprintf(stderr, "Please have a consistent type of indentation\n");
				}
			}

			break;
		case '#':
			// Multi-line comment -> #:<comment>:#
			if (nextcharadv(&vm->parser.lexer, ':'))
			{
				int nested = 0;

				while (nextchar(&vm->parser.lexer) != '\0')
				{
					if (nextcharadv(&vm->parser.lexer, '#'))
					{
						if (nextcharadv(&vm->parser.lexer, ':'))
							nested++;
					}
					else if (nextcharadv(&vm->parser.lexer, ':'))
					{
						if (nextcharadv(&vm->parser.lexer, '#'))
							nested--;
					}

					if (nested < 0)
						break;

					advchar(&vm->parser.lexer);
				}

				// If we go here then this must be an unterminated comment
				fprintf(stderr, "Unterminated comment");
			}
			// Single-line comment -> #<comment>
			else
				while (nextchar(&vm->parser.lexer) != '\n' && nextchar(&vm->parser.lexer) != '\0')
					advchar(&vm->parser.lexer);

			break;
		default:
			if (isdigit(c)) // Reads number!
				// TODO: Reads hex? (low priority)
			{
				while (isdigit(nextchar(&vm->parser.lexer)))
					advchar(&vm->parser.lexer);

				if (nextcharadv(&vm->parser.lexer, '.'))
					if (isdigit(nextchar(&vm->parser.lexer)))
						while (isdigit(nextchar(&vm->parser.lexer)))
							advchar(&vm->parser.lexer);

				if (nextcharadv(&vm->parser.lexer, 'e') || nextcharadv(&vm->parser.lexer, 'E'))
				{
					nextcharadv(&vm->parser.lexer, '-');

					if (!isdigit(nextchar(&vm->parser.lexer)))
						// TODO: Gives a vm->parser.lexer error
						fprintf(stderr, "Unterminated scientific notation!");

					while (isdigit(nextchar(&vm->parser.lexer)))
						advchar(&vm->parser.lexer);
				}

				errno = 0;
				double num = strtod(vm->parser.lexer.tokenstart, NULL);

				if (errno == ERANGE)
					// TODO: Gives a vm->parser.lexer error
					fprintf(stderr, "Number literal is too large!");

				Token *t = mktoken(vm, TOKEN_NUMBER);
				t->value.type = VALUE_NUMBER;
				t->value.num = num;

				break;
			}
			else if (isalpha(c) || c == '_') // Reads name!
			{
				while (isalnum(nextchar(&vm->parser.lexer)) || nextchar(&vm->parser.lexer) == '_')
					advchar(&vm->parser.lexer);

				TokenType type = TOKEN_IDENTIFIER;

				// Check if the name is reserved word/keyword
				for (int i = 0; keywords[i].word != NULL; i++)
					if (memcmp(&vm->parser.lexer.tokenstart, keywords[i].word, keywords[i].len) == 0)
					{
						type = keywords[i].type;
						break;
					}

				Token *t = mktoken(vm, type);

				if (type == TOKEN_FALSE || type == TOKEN_TRUE)
				{
					t->value.type = VALUE_BOOLEAN;
					t->value.bool = (type == TOKEN_FALSE ? 0 : 1);
				}
				else if (type == TOKEN_IDENTIFIER)
				{
					char str[t->len+1];
					strncpy(str, t->start, t->len);
					str[t->len+1] = '\0';
					t->value.str = str;
					t->value.type = VALUE_IDENTIFIER;
				}

				break;
			}

			mktoken(vm, TOKEN_UNKNOWN);
		}

		advchar(&vm->parser.lexer);
	}

	mktoken(vm, TOKEN_EOF);
}
