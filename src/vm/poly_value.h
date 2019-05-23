#ifndef POLY_VALUE_H_
#define POLY_VALUE_H_

typedef enum
{
	VALUE_NUMBER,
	VALUE_BOOLEAN,
	VALUE_IDENTIFIER
} ValueType;

typedef struct
{
	ValueType type;
	union
	{
		char* str;
		double num;
		_Bool bool;
	};
} Value;

typedef struct
{
	const char* identifier;
	const Value* value;
} Variable;

#endif
