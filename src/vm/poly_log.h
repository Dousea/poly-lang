#ifndef POLY_LOG_H
#define POLY_LOG_H

#include <stdlib.h>

/*
UNIX color palette:
RED     "\x1B[31m"
GREEN   "\x1B[32m"
YELLOW  "\x1B[33m"
BLUE    "\x1B[34m"
MAGENTA "\x1B[35m"
CYAN    "\x1B[36m"
WHITE   "\x1B[37m"
NORMAL  "\x1B[0m"
*/

typedef enum
{
    POLY_MSG_API,
    POLY_MSG_LEX,
    POLY_MSG_PRS,
    POLY_MSG_VMA,
    POLY_MSG_MEM
} MessageType;

#define POLY_LOG_START(type) \
    printf("\x1B[1;%dm[" #type "] ", (31 + POLY_MSG_ ## type));

#define POLY_LOG(fmt, args...) \
    printf(fmt, ## args);

#define POLY_LOG_END \
    printf("\x1B[0m");

#define POLY_IMM_LOG(type, fmt, args...) \
    printf("\x1B[1;%dm[" #type "] " fmt "\x1B[0m", (31 + POLY_MSG_##type), ## args);

#endif