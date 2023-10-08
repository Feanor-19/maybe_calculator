#ifndef SPU_H
#define SPU_H

#include "../common/onegin.h"
#include "../common/commands.h"

enum SPUError
{
    SPU_ERROR_NO_ERROR                  = 0,
    SPU_ERROR_GET_IN_OUT_FILES_NAMES    = 1,
    SPU_ERROR_READ_INPUT_FILE           = 2,
    SPU_ERROR_IN_PROG                   = 3,
};

enum SPUExecCmdRes
{
    EXEC_CMD_RES_DEFAULT    = 0,
    EXEC_CMD_RES_HLT        = 1,
};

struct Input
{
    Text text;
    FileBuf file_buf;
    SPUError err;
};

//-------------------------------------------------------------------------------------------------------------

//! @brief Reads from input file using Onegin and returns text and buffer,
//! packed as struct.
//! @param [in] input_file_name Name of the input file.
//! @return Struct Input.
Input read_input_file(const char* input_file_name);

SPUError run_program(Input prog, int *prog_res);

SPUExecCmdRes exec_command(Stack *stk_p, const char *line, Command cmd);

void free_struct_input(Input input);

#define ASSERT_INPUT(input) do { assert(input.text.line_array); assert(input.file_buf.buf); } while (0)

#endif /* SPU_H */
