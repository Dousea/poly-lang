#ifndef POLY_VALUE_H_
#define POLY_VALUE_H_

// Maximum values inside VM's stack
#define MAX_STACK 128
// Maximum scopes
#define MAX_SCOPES 8
// Maximum locals inside a scope level
#define MAX_LOCALS 256

typedef enum
{
	VALUE_NUMBER,
	VALUE_BOOLEAN
} ValueType;

typedef struct
{
	ValueType type;
	union
	{
		double num;
		unsigned int bool : 1; // TODO: Research about the difference between bit-field and _Bool
	};
} Value;

typedef enum
{
	CODE_LITERAL_NUMBER,
	CODE_LITERAL_FALSE,
	CODE_LITERAL_TRUE,
	CODE_EOF
} Code;

#endif /* POLY_VALUE_H_ */
