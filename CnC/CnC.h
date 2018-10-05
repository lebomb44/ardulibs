#ifndef CNC_H
#define CNC_H

#define MAX_MSG_SIZE 200
#define MAX_ARG (MAX_MSG_SIZE/2)
#include <stdint.h>

// command line structure
typedef struct _cnc_t
{
    const char * cmd;
    const char * subCmd;
    void (*func)(int argc, char **argv);
    struct _cnc_t *next;
} cnc_t;

void cncInit(const char * node);
void cnc_hkName_set(const char * hkName);
const __FlashStringHelper * cnc_hkName_get(void);
void cnc_cmdGetName_set(const char * cmdGetName);
const __FlashStringHelper * cnc_cmdGetName_get(void);
void cnc_cmdSetName_set(const char * cmdSetName);
const __FlashStringHelper * cnc_cmdSetName_get(void);
void cnc_sepName_set(const char * sepName);
const __FlashStringHelper * cnc_sepName_get(void);
void cncPoll(void);
void cnc_Add(const char * cmd, const char * subCmd, void (*func)(int argc, char **argv));
void cnc_cmdGet_Add(const char * cmd, void (*func)(int argc, char **argv));
void cnc_cmdSet_Add(const char * cmd, void (*func)(int argc, char **argv));
void cnc_print_hk(const char * cmd, int value);
void cnc_print_hk_index(const char * cmd, int index, int value);
void cnc_print_cmdGet(const char * cmd, int value);
void cnc_print_cmdSet(const char * cmd, int value);
uint32_t cncStr2Num(char *str, uint8_t base);

#endif //CNC_H
