#ifndef POLY_CODE_H
#define POLY_CODE_H

#include "poly_value.h"

typedef enum
{
    INST_LITERAL,
    INST_GET_VALUE,

    INST_BIN_ADD,
    INST_BIN_SUB,
    INST_BIN_MUL,
    INST_BIN_DIV,
    
    INST_UN_NEG,
    
    INST_ASSIGN,

    INST_END,
} Instruction;

#define CODE_INST  0
#define CODE_VALUE 1

typedef struct
{
    unsigned char type;
    union
    {
        const Value* value;
        Instruction inst;
    };
} Code;

#endif