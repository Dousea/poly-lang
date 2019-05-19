#ifndef SRC_VM_POLY_VALUE_H_
#define SRC_VM_POLY_VALUE_H_

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
	char* identifier;
	int len;
	Value *value;
} Variable;

#endif
