#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdint.h>

#include "..\..\common\common.h"

//---------------------------------------------------------------------------------------

const size_t MAX_ONE_WRITE_TO_OUT_LENGTH = 100; // сколько максимум можно записать за один раз
const size_t DEFAULT_TEXT_BUF_SIZE = 10;

//---------------------------------------------------------------------------------------

/*
enum DisasmStatus
{
    DISASM_STATUS_OK = 0,

    DISASM_STATUS_ERROR_GET_IN_OUT_FILES_NAMES,
    DISASM_STATUS_ERROR_READ_INPUT_FILE,
    DISASM_STATUS_ERROR_OPEN_OUTPUT_FILE,
    DISASM_STATUS_ERROR_MEM_ALLOC_ERROR,
    DISASM_STATUS_ERROR_INP_FILE_HEADER_CORRUPTED,
    DISASM_STATUS_ERROR_INP_FILE_CORRUPTED,
    DISASM_STATUS_ERROR_UNKOWN_CMD,
};
*/

#define DEF_STATUS(name, id, message) DISASM_STATUS_##name = id,
enum DisasmStatus
{
    #include "disasm_statuses.h"
};
#undef DEF_STATUS

#define DEF_STATUS(name, id, message) message,
const char *disassembler_error_messages[] =
{
    #include "disasm_statuses.h"
    "FICTIONAL MESSAGE!"
};
#undef DEF_STATUS

struct Binary
{
    int8_t *bin_arr = NULL;                 //< array of bytes from binary file
    size_t binary_size = 0;                 //< size of bin_arr
    size_t ip = 0;
    DisasmStatus err = DISASM_STATUS_OK;
};

struct Output
{
    char *text_buf = NULL;
    size_t curr_text_buf_size = 0;
    char *curr_char_ptr = NULL;
    DisasmStatus err = DISASM_STATUS_OK;
};

//---------------------------------------------------------------------------------------

Binary read_binary_file( const char *input_file_name );

DisasmStatus realloc_output( Output* out_ptr );

DisasmStatus write_chars_to_output( Output *out_ptr, char* char_arr, size_t nchars );

Output disassemble( Binary binary );

DisasmStatus write_to_output_file( Output out, const char *output_file_name );

void free_binary( Binary * bin_ptr );

void print_disasm_error_message( DisasmStatus err );

//TODO - придумать как передавать ошибку от write_chars_to_output
#define WRITE(out_ptr, ...) do {                                                    \
    char char_buf[MAX_ONE_WRITE_TO_OUT_LENGTH] = {};                                \
    write_chars_to_output( (out_ptr), char_buf, sprintf( char_buf, __VA_ARGS__) );  \
} while (0)

#define CHECK_ERR_(err) do{if ( (err) ) { print_disasm_error_message((err)); return (err); }} while(0)

#endif /* DISASSEMBLER_H */
