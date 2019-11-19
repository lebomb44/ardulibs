#ifndef CNC_H
#define CNC_H

#define MAX_MSG_SIZE 200
#define MAX_ARG (MAX_MSG_SIZE/2)
#include <stdint.h>
#include <Arduino.h>

// command line structure
typedef struct _cnc_t
{
    const char * cmd;
    const char * subCmd;
    void (*func)(int argc, char **argv);
    struct _cnc_t *next;
} cnc_t;

void cncInit(const char * node, HardwareSerial * serial_);
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
void cnc_print_hk_bool(const char * cmd, bool value);
void cnc_print_hk_u32(const char * cmd, uint32_t value);
void cnc_print_hk_index_float(const char * cmd, int index, float value);
void cnc_print_hk_temp_sensor(const char * cmd, uint8_t * sensor, float value);
void cnc_print_cmdGet_bool(const char * cmd, bool value);
void cnc_print_cmdGet_u32(const char * cmd, uint32_t value);
void cnc_print_cmdGet_tbd(const char * cmd);
void cnc_print_cmdSet_bool(const char * cmd, bool value);
uint32_t cncStr2Num(char *str, uint8_t base);
HardwareSerial * cnc_Serial_get(void);

#endif //CNC_H

