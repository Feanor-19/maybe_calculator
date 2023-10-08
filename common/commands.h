#ifndef COMMANDS_H
#define COMMANDS_H

enum Command
{
    CMD_UNKNOWN = 0,
    CMD_PUSH    = 1,
    CMD_ADD     = 2,
    CMD_SUB     = 3,
    CMD_MUL     = 4,
    CMD_DIV     = 5,
    CMD_IN      = 6,
    CMD_OUT     = 7,
    CMD_HLT     = 8,
};

//--- BINARY FILE HEADER
const char SIGN[4]  = {'S', 'F', '1', '9'};
const int VERSION   = 2;
const size_t NUM_OF_BYTES_FOR_BIN_LEN = 4;
const size_t HEADER_SIZE_IN_BYTES = sizeof(SIGN) + sizeof(VERSION) + sizeof(NUM_OF_BYTES_FOR_BIN_LEN);
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
