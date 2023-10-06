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
};

struct Input
{
    Text text;
    FileBuf file_buf;
    AssemblerError err;
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

const size_t commands_list_len = sizeof(commands_list)/sizeof(commands_list[0]);

//-------------------------------------------------------------------------------------------------------------

//! @brief Reads from input file using Onegin and returns text and buffer,
//! packed as struct.
//! @param [in] input_file_name Name of the input file.
//! @return Struct Input.
Input read_input_file(const char* input_file_name);

AssemblerError translate_and_write_to_output(Input input, const char *output_file_name);

//! @details Recieves string consisting of command's name and its argument (if needed),
//! returns corresponding element from enum Command and sets *cmd_end_ptr to the index
//! of the first char after the command.
//! @note Given string must not contain '\n', it must end just with '\0'.
//! @param [in] string String to parse.
//! @param [out] cmd_end_ptr Pointer to size_t variable.
//! @return Element of enum Command.
Command get_command(const char *str, size_t *cmd_end_ptr);

//! @brief Frees memory, allocated for input.text and
//! input.file_buf.
//! @param [in] input Struct input to free its elements.
void free_struct_input_file(Input input);

#define ASSERT_INPUT(input) do { assert(input.text.line_array); assert(input.file_buf.buf); } while (0)

#endif /* ASSEMBLER_H */
