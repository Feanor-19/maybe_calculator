#include <stdio.h>

#include "config_asm.h"

#include "assembler.h"

int main(int argc, const char *argv[])
{
    Config cfg = get_config(argc, argv);
    if (cfg.error)
    {
        print_cfg_error_message(stderr, cfg.error);
        return ASM_ERROR_GET_IN_OUT_FILES_NAMES;
    }

    print_config(stdout, cfg);

    const char *input_file_name = cfg.data_source;
    const char *output_file_name = cfg.output_destination;

    InputFile input_file = read_input_file(input_file_name);
    if (input_file.err) return input_file.err;





    free_struct_input_file(input_file);
}

InputFile read_input_file(const char* input_file_name)
{
    InputFile input_file = {};

    OneginErrorCodes err = ONEGIN_ERROR_NO;
    FileBuf file_buf = read_file_to_buf(input_file_name, &err);
    if (err)
    {
        print_onegin_error_message(err);
        input_file.err = ASM_ERROR_READ_INPUT_FILE;
        return input_file;
    }

    Text text = parse_buf_to_text(file_buf);

    input_file.text = text;
    input_file.file_buf = file_buf;
    return input_file;
}

void free_struct_input_file(InputFile input_file)
{
    assert(input_file.text.line_array);
    assert(input_file.file_buf.buf);

    free(input_file.file_buf.buf);
    input_file.file_buf.buf = NULL;
    input_file.file_buf.buf_size = 0;

    free(input_file.text.line_array);
    input_file.text.line_array = NULL;
    input_file.text.nLines = 0;
}
