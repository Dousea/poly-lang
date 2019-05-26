#ifndef POLY_CODE_H_
#define POLY_CODE_H_

#include "poly_value.h"

typedef enum poly_Instruction
{
    POLY_INST_LITERAL,
    POLY_INST_GET_VALUE,

    POLY_INST_BIN_ADD,
    POLY_INST_BIN_SUB,
    POLY_INST_BIN_MUL,
    POLY_INST_BIN_DIV,
    POLY_INST_BIN_MOD,
    POLY_INST_BIN_POW,
    
    POLY_INST_UN_NEG,
    
    POLY_INST_ASSIGN,

    POLY_INST_END,
} poly_Instruction;

#define POLY_CODE_INST  0
#define POLY_CODE_VALUE 1

typedef struct poly_Code
{
    unsigned char type;
    union
    {
        poly_Value *val;
        poly_Instruction inst;
    };
} poly_Code;

#endif