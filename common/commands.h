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
