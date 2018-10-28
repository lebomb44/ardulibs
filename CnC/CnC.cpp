#include <avr/pgmspace.h>
#include "Arduino.h"
#include "CnC.h"

// linked list for command table
static cnc_t *cmd_tbl_list;
static const char * cnc_node;
static const char * cnc_hkName;
static const char * cnc_cmdGetName;
static const char * cnc_cmdSetName;
static const char * cnc_sepName;

/**************************************************************************/
/*!
    Parse the command line. This function tokenizes the command input, then
    searches for the command table entry associated with the commmand. Once found,
    it will jump to the corresponding function.
*/
/**************************************************************************/
void cnc_parse(char *cmd)
{
    uint8_t argc, i = 0;
    char *argv[MAX_ARG] = {NULL};
    char buf[50] = {0};
    cnc_t *cmd_entry = NULL;

    fflush(stdout);

    // parse the command line statement and break it up into space-delimited
    // strings. the array of strings will be saved in the argv array.
    argv[i] = strtok(cmd, " ");
    do
    {
        argv[++i] = strtok(NULL, " ");
    } while ((i < (MAX_MSG_SIZE/2)) && (argv[i] != NULL));

    if(0 == argv[0])
    {
        return;
    }

    // save off the number of arguments for the particular command.
    argc = i;
    if(2 > argc) { return; }

    // parse the command table for valid command. used argv[0] which is the
    // actual command name typed in at the prompt
    for (cmd_entry = cmd_tbl_list; cmd_entry != NULL; cmd_entry = cmd_entry->next)
    {
        if (0 == strncmp_P(argv[0], cnc_node, strnlen_P(cnc_node, 50)))
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

/**************************************************************************/
/*!
    This function processes the individual characters typed into the command
    prompt. It saves them off into the message buffer unless its a "backspace"
    or "enter" key. 
*/
/**************************************************************************/
void cnc_handler()
{
    // command line message buffer and pointer
    static char msg[MAX_MSG_SIZE] = {0};
    static uint8_t nbChar = 0;

    char c = Serial.read();

    switch (c)
    {
    case '\r':
    case '\n':
        // terminate the msg and reset the msg ptr. then send
        // it to the handler for processing.
        msg[nbChar] = '\0';
        cnc_parse(msg);
        nbChar = 0;
        break;
    default:
        // normal character entered. add it to the buffer
        //Serial.print(c);
        if(MAX_MSG_SIZE-1 > nbChar)
        {
            msg[nbChar] = c;
            nbChar++;
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
    while (Serial.available())
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
void cncInit(const char * node)
{
    // init the command table
    cmd_tbl_list = NULL;

    cnc_node = node;
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
        Serial.println("ERROR: CnC struct malloc");
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
    Serial.print((__FlashStringHelper *)cnc_node); Serial.print(cnc_sepName_get());
    Serial.print((__FlashStringHelper *)cmd); Serial.print(cnc_sepName_get());
    Serial.print(cnc_hkName_get()); Serial.print(cnc_sepName_get());
    Serial.println(value, DEC); Serial.flush();
}

void cnc_print_hk_u32(const char * cmd, uint32_t value)
{
    Serial.print((__FlashStringHelper *)cnc_node); Serial.print(cnc_sepName_get());
    Serial.print((__FlashStringHelper *)cmd); Serial.print(cnc_sepName_get());
    Serial.print(cnc_hkName_get()); Serial.print(cnc_sepName_get());
    Serial.println(value, DEC); Serial.flush();
}

void cnc_print_hk_index_float(const char * cmd, int index, float value)
{
    Serial.print((__FlashStringHelper *)cnc_node); Serial.print(cnc_sepName_get());
    Serial.print((__FlashStringHelper *)cmd); Serial.print(cnc_sepName_get());
    Serial.print(cnc_hkName_get()); Serial.print(cnc_sepName_get());
    Serial.print(index, DEC); Serial.print(cnc_sepName_get());
    Serial.println(value, DEC); Serial.flush();
}

void cnc_print_cmdGet_bool(const char * cmd, bool value)
{
    Serial.print((__FlashStringHelper *)cnc_node); Serial.print(cnc_sepName_get());
    Serial.print((__FlashStringHelper *)cmd); Serial.print(cnc_sepName_get());
    Serial.print(cnc_cmdGetName_get()); Serial.print(cnc_sepName_get());
    Serial.println(value, DEC); Serial.flush();
}

void cnc_print_cmdGet_u32(const char * cmd, uint32_t value)
{
    Serial.print((__FlashStringHelper *)cnc_node); Serial.print(cnc_sepName_get());
    Serial.print((__FlashStringHelper *)cmd); Serial.print(cnc_sepName_get());
    Serial.print(cnc_cmdGetName_get()); Serial.print(cnc_sepName_get());
    Serial.println(value, DEC); Serial.flush();
}

void cnc_print_cmdSet_bool(const char * cmd, bool value)
{
    Serial.print((__FlashStringHelper *)cnc_node); Serial.print(cnc_sepName_get());
    Serial.print((__FlashStringHelper *)cmd); Serial.print(cnc_sepName_get());
    Serial.print(cnc_cmdSetName_get()); Serial.print(cnc_sepName_get());
    Serial.println(value, DEC); Serial.flush();
}

