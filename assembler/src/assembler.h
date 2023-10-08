#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "../common/onegin.h"
#include "../common/commands.h"

enum AssemblerError
{
    ASM_ERROR_NO_ERROR                  = 0,
    ASM_ERROR_GET_IN_OUT_FILES_NAMES    = 1,
    ASM_ERROR_READ_INPUT_FILE           = 2,
    ASM_ERROR_CANT_OPEN_OUTPUT_FILE     = 3,
    ASM_ERROR_MEM_ALLOC                 = 4,
    ASM_ERROR_UNKOWN_COMMAND            = 5,
    ASM_ERROR_CMD_ARG                   = 6,
};

struct Input
{
    Text text;
    FileBuf file_buf;
    AssemblerError err;
};

struct BinOut
{
    char *bin_arr;      //< Array of bytes - machine code.
    size_t bin_arr_len; //< Length of non-empty part of arr.
    AssemblerError err; //< Holds current error state.
};

struct CmdArg
{
    char cmd_byte;      //< 5 younger bits contain command, 3 older - type of arg
    int arg;
    size_t arg_size;    //< Number of bytes in arg which are actual informaton (arg_size <= sizeof(int))
    AssemblerError err; //< //< Holds current error state.
};

//-------------------------------------------------------------------------------------------------------------

//! @note MUST BE IN SYNC WITH ENUM COMMANDS!
const char *commands_list[] =
{
    "",
    "push",
    "add",
    "sub",
    "mul",
    "div",
    "in",
    "out",
    "hlt"
};

//! @note MUST BE IN SYNC WITH ENUM COMMANDS!
const int command_needs_arg[] =
{
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

const size_t commands_list_len = sizeof(commands_list)/sizeof(commands_list[0]);

//! @brief Everything between this symbol and the end of line is considered as a comment.
const char COMMENT_SYMB = ';';

//-------------------------------------------------------------------------------------------------------------

//! @brief Reads from input file using Onegin and returns text and buffer,
//! packed as struct.
//! @param [in] input_file_name Name of the input file.
//! @return Struct Input.
Input read_input_file(const char* input_file_name);

//! @brief Does some preprocessing, changing lines in input.
//! @details Cuts off commentaries.
//! @param [in] input Input to preprocess.
void preprocess_input(Input input);

BinOut translate_to_binary(Input input);

AssemblerError write_bin_to_output(BinOut bin_out, const char *output_file_name);

//! @details Recieves string consisting of command's name and its argument (if needed),
//! returns corresponding element from enum Command and sets *cmd_end_ptr to the index
//! of the first char after the command.
//! @note Given string must not contain '\n', it must end just with '\0'.
//! @param [in] string String to parse.
//! @param [out] cmd_end_ptr Pointer to size_t variable.
//! @return Element of enum Command.
Command get_command(const char *str, size_t *cmd_end_ptr);

CmdArg get_arg(Command cmd, const char *line, size_t cmd_end);

//! @brief Frees memory, allocated for input.text and
//! input.file_buf.
//! @param [in] input Struct input to free its elements.
void free_struct_input(Input input);

void free_struct_bin_out(BinOut bin_out);

#define ASSERT_INPUT(input) do { assert(input.text.line_array); assert(input.file_buf.buf); } while (0)

#endif /* ASSEMBLER_H */
