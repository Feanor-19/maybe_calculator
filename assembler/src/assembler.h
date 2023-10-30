#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "../../common/onegin.h"
#include "../../common/common.h"

#include <stdint.h>

//! @brief Everything between this symbol and the end of line is considered as a comment.
const char COMMENT_SYMB = ';';
const size_t LABEL_NAME_MAX_LEN = 50;
const size_t MAX_LABELS_COUNT = 100; //TODO - поменять массив меток на динамический, может быть список?

//-------------------------------------------------------------------------------------------------------------

#define DEF_STATUS(name, id, message) ASM_STATUS_##name = id,
enum AssemblerStatus
{
    #include "asm_statuses.h"
};
#undef DEF_STATUS

#define DEF_STATUS(name, id, message) message,
const char *assembler_error_messages[] =
{
    #include "asm_statuses.h"
    "FICTIONAL MESSAGE!"
};
#undef DEF_STATUS

struct Input
{
    Text text;
    FileBuf file_buf;
    AssemblerStatus err;
};

struct BinOut
{
    int8_t *bin_arr;        //< Array of bytes - machine code.
    size_t bin_arr_len;     //< Length of non-empty part of arr.
    AssemblerStatus err;    //< Holds current error state.
};

//TODO - стоит ли сделать union?
struct CmdArg
{
    immediate_const_t im_const;             //< Is used if needed.
    char label_name[LABEL_NAME_MAX_LEN];    //< Is used if needed.
    int8_t info_byte;                       //< See common.h ("Структура байта с информацией") for details.
    AssemblerStatus err;                    //< Holds current error state.
};

struct Label
{
    char name[LABEL_NAME_MAX_LEN];  //< Label's name.
    size_t bin_arr_ind;             //< Position of the label in the bin_arr.
};

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



AssemblerStatus write_bin_to_output(BinOut bin_out, const char *output_file_name);

//! @details Recieves string consisting of command's name and its argument (if needed),
//! returns corresponding element from enum Command and sets *cmd_end_ptr to the index
//! of the first char after the command.
//! @note Given string must not contain '\n', it must end just with '\0'.
//! @param [in] string String to parse.
//! @param [out] cmd_end_ptr Pointer to size_t variable.
//! @return Element of enum Command.
Command get_command(char *str, size_t *cmd_end_ptr);

CmdArg get_arg(Command cmd, const char *arg);

void print_asm_error_message(AssemblerStatus err);

//! @brief Frees memory, allocated for input.text and
//! input.file_buf.
//! @param [in] input Struct input to free its elements.
void free_struct_input(Input input);

void free_struct_bin_out(BinOut bin_out);

#define ASSERT_INPUT_(input) do { assert(input.text.line_array); assert(input.file_buf.buf); } while (0)

#define CHECK_ERR_(err) do{if ( (err) ) { print_asm_error_message((err)); return (err); }} while(0)

#endif /* ASSEMBLER_H */
