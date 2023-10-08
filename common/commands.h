#ifndef COMMANDS_H
#define COMMANDS_H

enum Command
{
    CMD_UNKNOWN = 0,
    CMD_PUSH    = 1,
    CMD_POP     = 2,
    CMD_ADD     = 3,
    CMD_SUB     = 4,
    CMD_MUL     = 5,
    CMD_DIV     = 6,
    CMD_IN      = 7,
    CMD_OUT     = 8,
    CMD_HLT     = 9,
};

//--- BINARY FILE HEADER
const char SIGN[4]  = {'S', 'F', '1', '9'};
const int VERSION   = 3;
const size_t NUM_OF_BYTES_FOR_BIN_LEN = 4;
const size_t HEADER_SIZE_IN_BYTES = sizeof(SIGN) + sizeof(VERSION) + NUM_OF_BYTES_FOR_BIN_LEN;
//---

const char *registers[] =
{
    "rax",
    "rbx",
    "rcx",
    "rdx"
};

const size_t register_name_len = 3;
const size_t registers_len = sizeof(registers)/sizeof(registers[0]);

const char bit_immediate_const  = 1 << 5;
const char bit_register         = 1 << 6;

#endif /* COMMANDS_H */
