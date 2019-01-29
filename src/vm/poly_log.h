#ifndef POLY_LOG_H
#define POLY_LOG_H

typedef enum
{
    POLY_MSG_API,
    POLY_MSG_LEX,
    POLY_MSG_PRS,
    POLY_MSG_MEM
} MessageType;

#define POLY_LOG_START(type) \
    printf("\033[1;%dm[" #type "] ", (31 + POLY_MSG_ ## type));

#define POLY_LOG(fmt, args...) \
    printf(fmt, ## args);

#define POLY_LOG_END \
    printf("\033[0m");

#define POLY_IMM_LOG(type, fmt, args...) { \
    POLY_LOG_START(type); \
    POLY_LOG(fmt, ## args); \
    POLY_LOG_END; \
}

#endif