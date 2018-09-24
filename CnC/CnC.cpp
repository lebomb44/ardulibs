#include <avr/pgmspace.h>
#include "Arduino.h"
#include "CnC.h"

// command line message buffer and pointer
static uint8_t msg[MAX_MSG_SIZE];
static uint8_t *msg_ptr;

// linked list for command table
static cnc_t *cmd_tbl_list;


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

    // parse the command table for valid command. used argv[0] which is the
    // actual command name typed in at the prompt
    for (cmd_entry = cmd_tbl_list; cmd_entry != NULL; cmd_entry = cmd_entry->next)
    {

        if (!strcmp(argv[0], cmd_entry->cmd))
        {
            cmd_entry->func(argc, argv);
            return;
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
    char c = Serial.read();

    switch (c)
    {
    case '\r':
    case '\n':
        // terminate the msg and reset the msg ptr. then send
        // it to the handler for processing.
        *msg_ptr = '\0';
        cnc_parse((char *)msg);
        msg_ptr = msg;
        break;
    default:
        // normal character entered. add it to the buffer
        //Serial.print(c);
        *msg_ptr++ = c;
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
void cncInit(void)
{
    // init the msg ptr
    msg_ptr = msg;

    // init the command table
    cmd_tbl_list = NULL;
}


/**************************************************************************/
/*!
    Add a command to the command table. The commands should be added in
    at the setup() portion of the sketch. 
*/
/**************************************************************************/
void cncAdd(const char *name, void (*func)(int argc, char **argv))
{
    if(50 <= strnlen(name, 50))
    {
        return;
    }

    // alloc memory for command struct
    cnc_t *cmd_tbl = (cnc_t *)malloc(sizeof(cnc_t));
    if(NULL == cmd_tbl)
    {
        Serial.println("ERROR: CnC struct malloc");
        return;
    }

    // fill out structure
    cmd_tbl->cmd = name;
    cmd_tbl->func = func;
    cmd_tbl->next = cmd_tbl_list;
    cmd_tbl_list = cmd_tbl;
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

