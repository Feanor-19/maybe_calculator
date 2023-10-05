#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "../common/onegin.h"

struct InputFile
{
    Text text;
    FileBuf file_buf;
    AssemblerError err;
};

enum AssemblerError
{
    ASM_ERROR_NO_ERROR                  = 0,
    ASM_ERROR_GET_IN_OUT_FILES_NAMES    = 1,
    ASM_ERROR_READ_INPUT_FILE           = 2,

};

//-------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------

InputFile read_input_file(const char* input_file_name);

void free_struct_input_file(InputFile input_file);

#endif /* ASSEMBLER_H */
