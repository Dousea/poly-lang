#ifndef VALUE_H_
#define VALUE_H_

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
		unsigned int bool : 1;
	};
} Value;

#endif /* VALUE_H_ */
