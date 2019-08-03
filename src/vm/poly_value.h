#ifndef POLY_VALUE_H_
#define POLY_VALUE_H_

typedef enum poly_ValueType
{
	POLY_VAL_NUM,
	POLY_VAL_BOOL,
	POLY_VAL_ID
} poly_ValueType;

typedef char* poly_String;
typedef double poly_Number;
typedef _Bool poly_Boolean;

typedef struct poly_Value
{
	poly_ValueType type;
	union
	{
		poly_String str;
		poly_Number num;
		poly_Boolean bool;
	};
} poly_Value;

typedef struct poly_Variable
{
	const char *id;
	poly_Value *val;
} poly_Variable;

#endif
