#include <stdio.h>

#include "poly_vm.h"
#include "poly_log.h"

// Checks if the lexical token stream is at an allowable form and creates bytecodes
POLY_LOCAL void parse(VM *vm)
{
#ifdef POLY_DEBUG
	POLY_IMM_LOG(PRS, "Parsing...\n")
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
		POLY_LOG_START(PRS)
		POLY_LOG("Reading token 0x%02X", curtoken->type)

		if (curtoken->type == TOKEN_NUMBER)
			POLY_LOG(" with value of %f", curtoken->value.num)
		else if (curtoken->type == TOKEN_IDENTIFIER &&
		         curtoken->value.type == VALUE_BOOLEAN)
			POLY_LOG(" with value of %s", (curtoken->value.bool == 0 ? "false" : "true"))

		POLY_LOG("...\n")
		POLY_LOG_END
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
