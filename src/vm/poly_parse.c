#include <stdio.h>

#include "poly_vm.h"

// Checks if the lexical token stream is at an allowable form and creates bytecodes
POLY_LOCAL void parse(VM *vm)
{
#ifdef POLY_DEBUG
	printf("Parsing tokens..\n");
#endif

	// Current token that's being consumed
	Token *curtoken = vm->parser.tokenstream;
	// Previous token that already consumed
	Token *prevtoken = curtoken;
	// Current line position of token that's being consumed
	int curln = 1;

	while (curtoken->type != TOKEN_EOF)
	{
		printf("Reading token %d with value of %f\n",
				curtoken->type,
				(curtoken->value.type == VALUE_NUMBER ? curtoken->value.num : (double)curtoken->value.bool));

		switch (curtoken->type)
		{
		case TOKEN_NEWLINE:
			curln++; break;
		case TOKEN_INDENT:
			break;
		case TOKEN_UNKNOWN:
			// TODO: Gives a parser error
			fprintf(stderr, "An unknown symbol is found\n"); break;
		default:

			break;
		}

		prevtoken = curtoken++;
	}
}
