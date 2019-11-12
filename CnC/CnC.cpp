#include <avr/pgmspace.h>
#include "Arduino.h"
#include "CnC.h"

// linked list for command table
static cnc_t * cmd_tbl_list = NULL;
static const char * cnc_node = NULL;
static const char * cnc_hkName = NULL;
static const char * cnc_cmdGetName = NULL;
static const char * cnc_cmdSetName = NULL;
static const char * cnc_sepName = NULL;
static const char * cnc_cmdvalName = NULL;
static const char * cnc_cmdindexName = NULL;
static char * cnc_msg = NULL;
static uint8_t cnc_msg_index = 0;

HardwareSerial * cnc_serial = NULL;

/**************************************************************************/
/*!
    Parse the command line. This function tokenizes the command input, then
    searches for the command table entry associated with the commmand. Once found,
    it will jump to the corresponding function.
*/
/**************************************************************************/
void cnc_parse(void)
{
    uint8_t argc, i = 0;
    char *argv[10] = {NULL};
    cnc_t *cmd_entry = NULL;

    //fflush(stdout);

    // parse the command line statement and break it up into space-delimited
    // strings. the array of strings will be saved in the argv array.
    argv[0] = strtok(cnc_msg, " ");
    if(NULL == argv[0])
    {
        return;
    }
    do
    {
        i++;
        argv[i] = strtok(NULL, " ");
    } while ((9 > i) && (NULL != argv[i]));

    // save off the number of arguments for the particular command.
    argc = i;
    if(2 < argc)
    {
        // parse the command table for valid command. used argv[0] which is the
        // actual command name typed in at the prompt
        if (0 == strncmp_P(argv[0], cnc_node, strnlen_P(cnc_node, 50)))
        {
            for (cmd_entry = cmd_tbl_list; cmd_entry != NULL; cmd_entry = cmd_entry->next)
            {
                if (0 == strncmp_P(argv[1], cmd_entry->cmd, strnlen_P(cmd_entry->cmd, 50)))
                {
                    if (0 == strncmp_P(argv[2], cmd_entry->subCmd, strnlen_P(cmd_entry->subCmd, 50)))
                    {
                        cmd_entry->func(argc, argv);
                        return;
                    }
                }
            }
        }
    }
    cnc_serial->print((__FlashStringHelper *)cnc_node); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print("unknown");
    for(i=0; i<argc; i++)
    {
        cnc_serial->print(cnc_sepName_get()); cnc_serial->print(argv[i]);
    }
    cnc_serial->println();
    cnc_serial->flush();
}

/**************************************************************************/
/*!
    This function processes the individual characters typed into the command
    prompt. It saves them off into the message buffer unless its a "backspace"
    or "enter" key. 
*/
/**************************************************************************/
void cnc_handler()
{
    char c = cnc_serial->read();

    switch (c)
    {
    case '\r':
    case '\n':
        // terminate the msg and reset the msg ptr. then send
        // it to the handler for processing.
        cnc_msg[cnc_msg_index] = '\0';
        cnc_parse();
        cnc_msg_index = 0;
        break;
    default:
        // normal character entered. add it to the buffer
        if((MAX_MSG_SIZE-1) > cnc_msg_index)
        {
            cnc_msg[cnc_msg_index] = c;
            cnc_msg_index++;
        }
        break;
    }
}

/**************************************************************************/
/*!
    This function should be set inside the main loop. It needs to be called
    constantly to check if there is any available input at the command prompt.
*/
/**************************************************************************/
void cncPoll()
{
    while (cnc_serial->available())
    {
        cnc_handler();
    }
}

/**************************************************************************/
/*!
    Initialize the command line interface. This sets the terminal speed and
    and initializes things. 
*/
/**************************************************************************/
void cncInit(const char * node, HardwareSerial * serial_)
{
    cnc_serial = serial_;

    // init the command table
    cmd_tbl_list = NULL;

    cnc_node = node;

    cnc_msg_index = 0;
    cnc_msg = malloc(MAX_MSG_SIZE);
    if(NULL == cnc_msg)
    {
        cnc_serial->println("ERROR: CnC msg malloc");
        return;
    }
}

void cncInit(const char * node)
{
    cncInit(node, &Serial);
}

void cnc_hkName_set(const char * hkName)
{
    cnc_hkName = hkName;
}

const __FlashStringHelper * cnc_hkName_get(void)
{
    return (const __FlashStringHelper *) cnc_hkName;
}

void cnc_cmdGetName_set(const char * cmdGetName)
{
    cnc_cmdGetName = cmdGetName;
}

const __FlashStringHelper * cnc_cmdGetName_get(void)
{
    return (const __FlashStringHelper *) cnc_cmdGetName;
}

void cnc_cmdSetName_set(const char * cmdSetName)
{
    cnc_cmdSetName = cmdSetName;
}

const __FlashStringHelper * cnc_cmdSetName_get(void)
{
    return (const __FlashStringHelper *) cnc_cmdSetName;
}

void cnc_sepName_set(const char * sepName)
{
    cnc_sepName = sepName;
}

const __FlashStringHelper * cnc_sepName_get(void)
{
    return (const __FlashStringHelper *) cnc_sepName;
}

/**************************************************************************/
/*!
    Add a command to the command table. The commands should be added in
    at the setup() portion of the sketch. 
*/
/**************************************************************************/
void cnc_Add(const char * cmd, const char * subCmd, void (*func)(int argc, char **argv))
{
    // alloc memory for command struct
    cnc_t *cmd_tbl = (cnc_t *)malloc(sizeof(cnc_t));
    if(NULL == cmd_tbl)
    {
        cnc_serial->println("ERROR: CnC struct malloc");
        return;
    }

    // fill out structure
    cmd_tbl->cmd = cmd;
    cmd_tbl->subCmd = subCmd;
    cmd_tbl->func = func;
    cmd_tbl->next = cmd_tbl_list;
    cmd_tbl_list = cmd_tbl;
}

void cnc_cmdGet_Add(const char * cmd, void (*func)(int argc, char **argv))
{
    cnc_Add(cmd, cnc_cmdGetName, func);
}

void cnc_cmdSet_Add(const char * cmd, void (*func)(int argc, char **argv))
{
    cnc_Add(cmd, cnc_cmdSetName, func);
}

/**************************************************************************/
/*!
    Convert a string to a number. The base must be specified, ie: "32" is a
    different value in base 10 (decimal) and base 16 (hexadecimal).
*/
/**************************************************************************/
uint32_t cncStr2Num(char *str, uint8_t base)
{
    return strtoul(str, NULL, base);
}

void cnc_print_hk_bool(const char * cmd, bool value)
{
    cnc_serial->print((__FlashStringHelper *)cnc_node); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print((__FlashStringHelper *)cmd); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print(cnc_hkName_get()); cnc_serial->print(cnc_sepName_get());
    cnc_serial->println(value, DEC); cnc_serial->flush();
}

void cnc_print_hk_u32(const char * cmd, uint32_t value)
{
    cnc_serial->print((__FlashStringHelper *)cnc_node); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print((__FlashStringHelper *)cmd); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print(cnc_hkName_get()); cnc_serial->print(cnc_sepName_get());
    cnc_serial->println(value, DEC); cnc_serial->flush();
}

void cnc_print_hk_index_float(const char * cmd, int index, float value)
{
    cnc_serial->print((__FlashStringHelper *)cnc_node); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print((__FlashStringHelper *)cmd); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print(index, DEC); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print(cnc_hkName_get()); cnc_serial->print(cnc_sepName_get());
    cnc_serial->println(value, DEC); cnc_serial->flush();
}

void cnc_print_hk_temp_sensor(const char * cmd, uint8_t * sensor, float value)
{
    cnc_serial->print((__FlashStringHelper *)cnc_node); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print((__FlashStringHelper *)cmd); cnc_serial->print(cnc_sepName_get());
    for(uint8_t i=0; i<8; i++) { if(16>sensor[i]) { cnc_serial->print("0"); } cnc_serial->print(sensor[i], HEX); }
    cnc_serial->print(cnc_sepName_get());
    cnc_serial->print(cnc_hkName_get()); cnc_serial->print(cnc_sepName_get());
    cnc_serial->println(value, DEC); cnc_serial->flush();
}


void cnc_print_cmdGet_bool(const char * cmd, bool value)
{
    cnc_serial->print((__FlashStringHelper *)cnc_node); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print((__FlashStringHelper *)cmd); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print(cnc_cmdGetName_get()); cnc_serial->print(cnc_sepName_get());
    cnc_serial->println(value, DEC); cnc_serial->flush();
}

void cnc_print_cmdGet_u32(const char * cmd, uint32_t value)
{
    cnc_serial->print((__FlashStringHelper *)cnc_node); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print((__FlashStringHelper *)cmd); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print(cnc_cmdGetName_get()); cnc_serial->print(cnc_sepName_get());
    cnc_serial->println(value, DEC); cnc_serial->flush();
}

void cnc_print_cmdGet_tbd(const char * cmd)
{
    cnc_serial->print((__FlashStringHelper *)cnc_node); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print((__FlashStringHelper *)cmd); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print(cnc_cmdGetName_get()); cnc_serial->print(cnc_sepName_get());
    cnc_serial->flush();
}

void cnc_print_cmdSet_bool(const char * cmd, bool value)
{
    cnc_serial->print((__FlashStringHelper *)cnc_node); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print((__FlashStringHelper *)cmd); cnc_serial->print(cnc_sepName_get());
    cnc_serial->print(cnc_cmdSetName_get()); cnc_serial->print(cnc_sepName_get());
    cnc_serial->println(value, DEC); cnc_serial->flush();
}
