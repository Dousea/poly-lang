#ifndef POLY_VALUE_H_
#define POLY_VALUE_H_

typedef enum poly_ValueType
{
	POLY_VAL_NUM,
	POLY_VAL_BOOL,
	POLY_VAL_ID
} poly_ValueType;

typedef struct poly_Value
{
	poly_ValueType type;
	union
	{
		char *str;
		double num;
		_Bool bool;
	};
} poly_Value;

typedef struct poly_Variable
{
	const char *id;
	poly_Value *val;
} poly_Variable;

#endif
