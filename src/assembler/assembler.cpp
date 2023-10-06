#include <stdio.h>
#include <ctype.h>

#include "config_asm.h"

#include "assembler.h"

int main(int argc, const char *argv[])
{
    /*
    const char *str = "pu 12";
    size_t off = 0;
    Command cmd = get_command(str, &off);
    printf("!!!%d <%s>\n", cmd, str + off);
    if (cmd)
    {
        int x = 0;
        sscanf(str + off, "%d", &x);
        printf("!!!x: <%d>\n", x);
    }
    */

   /*
    char buf[] = "1234567890";
    size_t buf_ind = 0;
    buf_ind += sprintf(buf + buf_ind, "%s", "hi");
    buf_ind += sprintf(buf + buf_ind, "%s", " world");
    buf_ind += sprintf(buf + buf_ind, "%s", "!\n");
    printf("! buf = <%s>", buf);
    printf("@ buf + buf_ind = <%s>", buf + buf_ind);
    */


    Config cfg = get_config(argc, argv);
    if (cfg.error)
    {
        print_cfg_error_message(stderr, cfg.error);
        return ASM_ERROR_GET_IN_OUT_FILES_NAMES;
    }

    print_config(stdout, cfg);

    const char *input_file_name = cfg.data_source;
    const char *output_file_name = cfg.output_destination;

    Input input = read_input_file(input_file_name);
    if (input.err) return input.err;

    AssemblerError err = translate_and_write_to_output(input, output_file_name);
    //printf("~~~%d\n", err);
    if (err) return err;

    free_struct_input_file(input);

    fprintf(stdout, "%s is translated to %s!", input_file_name, output_file_name);

}

Input read_input_file(const char* input_file_name)
{
    assert(input_file_name);

    Input input = {};

    OneginErrorCodes err = ONEGIN_ERROR_NO;
    FileBuf file_buf = read_file_to_buf(input_file_name, &err);
    if (err)
    {
        print_onegin_error_message(err);
        input.err = ASM_ERROR_READ_INPUT_FILE;
        return input;
    }

    Text text = parse_buf_to_text(file_buf);

    input.text = text;
    input.file_buf = file_buf;
    return input;
}

inline void print_asm_error(long int line, const char *str)
{
    fprintf(stderr, "ERROR on line %ld! The line is:\n<%s>\n",
                    line, str);
}

AssemblerError translate_and_write_to_output(Input input, const char *output_file_name)
{
    ASSERT_INPUT(input);
    assert(output_file_name);

    // Примечание: считается, что всегда численное значение команды короче, чем ее словесное обозначение
    // (по количеству символов), а потому нам достаточно для транслированного текста того количества байтов,
    // которое было в изначальном
    char *buf = (char *) calloc(input.file_buf.buf_size, sizeof(char));
    if (!buf) return ASM_ERROR_MEM_ALLOC;
    size_t buf_ind = 0;

    /*
    printf("buf_file: <");
    for (size_t ind = 0; ind < input.file_buf.buf_size; ind++)
    {
        if (input.file_buf.buf[ind] == '\0') printf("*");
        else printf("%c", input.file_buf.buf[ind]);
    }
    printf(">\n");
    */

    for (unsigned long ind = 0; ind < input.text.nLines; ind++)
    {
        size_t cmd_end = 0;
        Command cmd = get_command(input.text.line_array[ind], &cmd_end);
        if (cmd == CMD_UNKNOWN)
        {
            print_asm_error(ind + 1, input.text.line_array[ind]);
            //printf("### <%s>\n", input.text.line_array[ind]);
            free(buf); // ???
            return ASM_ERROR_UNKOWN_COMMAND;
        }

        buf_ind += sprintf(buf + buf_ind, "%d", cmd);

        if (command_needs_arg[cmd])
        {
            int arg = 0;
            if ( sscanf(input.text.line_array[ind] + cmd_end, "%d", &arg) != 1 )
            {
                print_asm_error(ind + 1, input.text.line_array[ind]);
                free(buf); // ???
                return ASM_ERROR_CMD_ARG;
            }

            buf_ind += sprintf(buf + buf_ind, " %d", arg);
        }

        buf_ind += sprintf(buf + buf_ind, "\n");
    }

    FILE* out = fopen(output_file_name, "w");
    if (!out)
    {
        free(buf); // ???
        return ASM_ERROR_CANT_OPEN_OUTPUT_FILE;
    }

    fwrite(buf, sizeof(char), buf_ind, out);

    fclose(out);
    free(buf);
    return ASM_ERROR_NO_ERROR;
}

Command get_command(const char *str, size_t *cmd_end_ptr)
{
    assert(str);
    assert(cmd_end_ptr);

    for (size_t cmd_ind = 1; cmd_ind < commands_list_len; cmd_ind++)
    {
        size_t str_ind = 0;
        while (str[str_ind] == ' ' || str[str_ind] != '\0')
        {
            if (tolower(str[str_ind]) != tolower(commands_list[cmd_ind][str_ind]))
            {
                break;
            }

            str_ind++;
        }

        if ((str[str_ind] == ' ' || str[str_ind] == '\0') && commands_list[cmd_ind][str_ind] == '\0')
        {
            *cmd_end_ptr = str_ind;
            return (Command) cmd_ind;
        }
    }

    return CMD_UNKNOWN;
}

void free_struct_input_file(Input input)
{
    ASSERT_INPUT(input);

    free(input.file_buf.buf);
    input.file_buf.buf = NULL;
    input.file_buf.buf_size = 0;

    free(input.text.line_array);
    input.text.line_array = NULL;
    input.text.nLines = 0;
}
