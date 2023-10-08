#include <stdio.h>
#include <ctype.h>

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

    Input input = read_input_file(cfg.input_file_name);
    if (input.err) return input.err;

    preprocess_input(input);

    //AssemblerError err = translate_and_write_to_output(input, cfg.output_file_name);
    //if (err) return err;

    BinOut bin_out = translate_to_binary(input);
    if (bin_out.err) return bin_out.err;

    //---
    printf("bin:\n");
    for (size_t ind = 0; ind < bin_out.bin_arr_len; ind++)
    {
        printf("%d ", bin_out.bin_arr[ind]);
    }
    printf("\n");
    //---

    AssemblerError err = write_bin_to_output(bin_out, cfg.output_file_name);
    if (err) return err;

    free_struct_input(input);
    free_struct_bin_out(bin_out);

    fprintf(stdout, "%s is translated to %s!", cfg.input_file_name, cfg.output_file_name);
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

void preprocess_input(Input input)
{
    ASSERT_INPUT(input);

    for (unsigned long ind = 0; ind < input.text.nLines; ind++)
    {
        char *c_p = strchr(input.text.line_array[ind], COMMENT_SYMB);
        if (c_p)
        {
            *c_p = '\0';
        }
    }
}

inline void print_asm_error(unsigned long line, const char *str)
{
    fprintf(stderr, "ERROR on line %ld! The line is:\n<%s>\n",
                    line, str);
}

BinOut translate_to_binary(Input input)
{
    ASSERT_INPUT(input);

    BinOut bin_out = {};

    // Примечание: считается, что всегда численное значение команды короче, чем ее словесное обозначение
    // (по количеству символов), а потому нам достаточно для транслированного текста того количества байтов,
    // которое было в изначальном
    char *bin_arr = (char *) calloc(input.file_buf.buf_size, sizeof(char));
    if (!bin_arr)
    {
        bin_out.err = ASM_ERROR_MEM_ALLOC;
        return bin_out;
    }
    size_t bin_arr_ind = 0;

    for (unsigned long ind = 0; ind < input.text.nLines; ind++)
    {
        size_t cmd_end = 0;
        Command cmd = get_command(input.text.line_array[ind], &cmd_end);
        if (cmd == CMD_UNKNOWN)
        {
            print_asm_error(ind + 1, input.text.line_array[ind]);
            free(bin_arr); // ???
            bin_out.err = ASM_ERROR_UNKOWN_COMMAND;
            return bin_out;
        }

        bin_arr[bin_arr_ind] = (char) cmd;

        if (command_needs_arg[cmd])
        {
            // вызываем функцию, которая выясняет какой именно доп аргумент у команды,
            // соттветсвенно возвращает новое значение cmd_byte (в нем поменяны левые три бита)
            // размер в байтах аргумента и сам аргумент (который всегда int, но если размер аргумента
            // один байт, то читаем из инта только младший байт)

            CmdArg cmd_arg = get_arg(cmd, input.text.line_array[ind], cmd_end);
            if (cmd_arg.err)
            {
                bin_out.err = cmd_arg.err;
                return bin_out;
            }

            bin_arr[bin_arr_ind++] = cmd_arg.cmd_byte;

            if (cmd_arg.arg_size == 1)
            {
                bin_arr[bin_arr_ind] = (char) cmd_arg.arg;
            }
            else if (cmd_arg.arg_size == 4)
            {
                *((int *) (bin_arr + bin_arr_ind)) = (int) cmd_arg.arg;
                bin_arr_ind += 3; // тк после будет еще один bin_arr_ind
            }
        }

        bin_arr_ind++;
    }

    bin_out.bin_arr = bin_arr;
    bin_out.bin_arr_len = bin_arr_ind;
    bin_out.err = ASM_ERROR_NO_ERROR;

    return bin_out;
}

AssemblerError write_bin_to_output(BinOut bin_out, const char *output_file_name)
{
    FILE* out = fopen(output_file_name, "w");
    if (!out)
    {
        fclose(out); // ???
        return ASM_ERROR_CANT_OPEN_OUTPUT_FILE;
    }

    fwrite(bin_out.bin_arr, sizeof(char), bin_out.bin_arr_len, out);

    fclose(out);

    return ASM_ERROR_NO_ERROR;
}

/*
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

    for (unsigned long ind = 0; ind < input.text.nLines; ind++)
    {
        size_t cmd_end = 0;
        Command cmd = get_command(input.text.line_array[ind], &cmd_end);
        if (cmd == CMD_UNKNOWN)
        {
            print_asm_error(ind + 1, input.text.line_array[ind]);
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
*/

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

//! @brief Checks whether rgstr is a valid register's name or not.
//! Returns register's id or 0.
inline int check_reg_name(const char *rgstr)
{
    for ( size_t ind = 0; ind < registers_len; ind++ )
    {
        if ( strcmp(rgstr, registers[ind]) == 0 )
        {
            return 1;
        }
    }
    return 0;
}

inline CmdArg get_arg_push(Command cmd, const char *line, size_t cmd_end)
{
    assert(line);

    CmdArg cmd_arg = {};

    int immediate_const = 0;
    char rgstr[register_name_len] = "";
    size_t reg_id = 0;
    if ( sscanf(line + cmd_end, "%d", &immediate_const) == 1 )
    {
        cmd_arg.cmd_byte = ((char) cmd) | bit_immediate_const;
        cmd_arg.arg = immediate_const;
        cmd_arg.arg_size = sizeof(int);
        cmd_arg.err = ASM_ERROR_NO_ERROR;
    }
    else if ( sscanf(line + cmd_end, "%s", rgstr) == 1 && (reg_id = check_reg_name(rgstr)) )
    {
        cmd_arg.cmd_byte = ((char) cmd) | bit_register;
        cmd_arg.arg = (int) reg_id;
        cmd_arg.arg_size = sizeof(int);
        cmd_arg.err = ASM_ERROR_NO_ERROR;
    }
    else
    {
        cmd_arg.err = ASM_ERROR_CMD_ARG;
    }

    return cmd_arg;
}

CmdArg get_arg(Command cmd, const char *line, size_t cmd_end)
{
    assert(line);

    switch (cmd)
    {
    case CMD_PUSH:
        return get_arg_push(cmd, line, cmd_end);
        break;
    case CMD_ADD:
    case CMD_DIV:
    case CMD_HLT:
    case CMD_IN:
    case CMD_MUL:
    case CMD_OUT:
    case CMD_SUB:
    case CMD_UNKNOWN:
    default:
        assert(0 && "Default case in switch!");
        break;
    }

    return {};
}

void free_struct_input(Input input)
{
    ASSERT_INPUT(input);

    free(input.file_buf.buf);
    input.file_buf.buf = NULL;
    input.file_buf.buf_size = 0;

    free(input.text.line_array);
    input.text.line_array = NULL;
    input.text.nLines = 0;
}

void free_struct_bin_out(BinOut bin_out)
{
    if (bin_out.bin_arr) free(bin_out.bin_arr);
    bin_out.bin_arr_len = 0;
}
