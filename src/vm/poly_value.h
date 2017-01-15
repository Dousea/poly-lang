#ifndef SRC_VM_POLY_VALUE_H_
#define SRC_VM_POLY_VALUE_H_

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

#endif
