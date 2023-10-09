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

//! @note MUST BE IN SYNC WITH ENUM COMMANDS!
const char *commands_list[] =
{
    "",     //CMD_UNKNOWN
    "push", //CMD_PUSH
    "pop",  //CMD_POP
    "add",  //CMD_ADD
    "sub",  //CMD_SUB
    "mul",  //CMD_MUL
    "div",  //CMD_DIV
    "in",   //CMD_IN
    "out",  //CMD_OUT
    "hlt"   //CMD_HLT
};

//! @note MUST BE IN SYNC WITH ENUM COMMANDS!
const int command_needs_im_const_arg[] =
{
    0,  //CMD_UNKNOWN
    1,  //CMD_PUSH
    0,  //CMD_POP
    0,  //CMD_ADD
    0,  //CMD_SUB
    0,  //CMD_MUL
    0,  //CMD_DIV
    0,  //CMD_IN
    0,  //CMD_OUT
    0   //CMD_HLT
};

//! @note MUST BE IN SYNC WITH ENUM COMMANDS!
const int command_needs_register_arg[] =
{
    0,  //CMD_UNKNOWN
    1,  //CMD_PUSH
    1,  //CMD_POP
    0,  //CMD_ADD
    0,  //CMD_SUB
    0,  //CMD_MUL
    0,  //CMD_DIV
    0,  //CMD_IN
    0,  //CMD_OUT
    0   //CMD_HLT
};

//! @note MUST BE IN SYNC WITH ENUM COMMANDS!
const int command_needs_arg[] =
{
    command_needs_im_const_arg[0] || command_needs_register_arg[0],  //CMD_UNKNOWN
    command_needs_im_const_arg[1] || command_needs_register_arg[1],  //CMD_PUSH
    command_needs_im_const_arg[2] || command_needs_register_arg[2],  //CMD_POP
    command_needs_im_const_arg[3] || command_needs_register_arg[3],  //CMD_ADD
    command_needs_im_const_arg[4] || command_needs_register_arg[4],  //CMD_SUB
    command_needs_im_const_arg[5] || command_needs_register_arg[5],  //CMD_MUL
    command_needs_im_const_arg[6] || command_needs_register_arg[6],  //CMD_DIV
    command_needs_im_const_arg[7] || command_needs_register_arg[7],  //CMD_IN
    command_needs_im_const_arg[8] || command_needs_register_arg[8],  //CMD_OUT
    command_needs_im_const_arg[9] || command_needs_register_arg[9]   //CMD_HLT
};
const size_t commands_list_len = sizeof(commands_list)/sizeof(commands_list[0]);

//--- BINARY FILE HEADER
const char SIGN[4]  = {'S', 'F', '1', '9'};
const int VERSION   = 4;
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

const size_t immediate_const_size_in_bytes = sizeof(int);
const size_t register_id_size_in_bytes = sizeof(char);

//-------------------------------------------------------------------------------------------------------

#define sizearr(arr) sizeof(arr)/sizeof(arr[0])

static_assert(sizearr(commands_list) == sizearr(command_needs_arg),
                "commands_list's and command_needs_arg's sizes are not equal!");

static_assert(sizearr(command_needs_arg) == sizearr(command_needs_im_const_arg),
                "command_needs_arg's and command_needs_im_const_arg's sizes are not equal!");

static_assert(sizearr(command_needs_im_const_arg) == sizearr(command_needs_register_arg),
                "command_needs_im_const_arg's and command_needs_register_arg's sizes are snot equal!");


#undef sizearr

#endif /* COMMANDS_H */
