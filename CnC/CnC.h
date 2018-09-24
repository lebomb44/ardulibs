#ifndef CNC_H
#define CNC_H

#define MAX_MSG_SIZE 200
#define MAX_ARG (MAX_MSG_SIZE/2)
#include <stdint.h>

// command line structure
typedef struct _cnc_t
{
    char *cmd;
    void (*func)(int argc, char **argv);
    struct _cnc_t *next;
} cnc_t;

void cncInit(void);
void cncPoll(void);
void cncAdd(const char *name, void (*func)(int argc, char **argv));
uint32_t cncStr2Num(char *str, uint8_t base);

#endif //CNC_H

