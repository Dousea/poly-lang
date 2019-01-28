#include <stdio.h>

#include "poly_vm.h"

// Checks if the lexical token stream is at an allowable form and creates bytecodes
POLY_LOCAL void parse(VM *vm)
{
#ifdef POLY_DEBUG
	printf("[PARSING TOKENS]\n");
#endif

	// Current token that's being consumed
	Token *curtoken = vm->parser.tokenstream;
	// Previous token that already consumed
	Token *prevtoken = curtoken;
	// Current line position of token that's being consumed
	int curln = 1;

	while (curtoken->type != TOKEN_EOF)
	{
#ifdef POLY_DEBUG
		printf("Reading token %d", curtoken->type);

		if (curtoken->value.type == VALUE_NUMBER)
			printf(" with value of %f", curtoken->value.num);
		else if (curtoken->value.type == VALUE_BOOLEAN)
			printf(" with value of %s", (curtoken->value.bool == 0 ? "false" : "true"));

		printf("\n");
#endif

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
