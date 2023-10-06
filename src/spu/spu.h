#ifndef SPU_H
#define SPU_H

#include "../common/onegin.h"
#include "../common/commands.h"

enum SPUError
{
    SPU_ERROR_NO_ERROR                  = 0,
    SPU_ERROR_GET_IN_OUT_FILES_NAMES    = 1,
    SPU_ERROR_READ_INPUT_FILE           = 2,
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


#endif /* SPU_H */
