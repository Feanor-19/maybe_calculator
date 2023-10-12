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
    CHECK_ERR_(input.err);

    preprocess_input(input);

    BinOut bin_out = translate_to_binary(input);
    CHECK_ERR_(bin_out.err);

    //---
    /*
    printf("bin:\n");
    for (size_t ind = 0; ind < bin_out.bin_arr_len; ind++)
    {
        printf("%d ", bin_out.bin_arr[ind]);
    }
    printf("\n");
    */
    //---

    AssemblerError err = write_bin_to_output(bin_out, cfg.output_file_name);
    CHECK_ERR_(err);

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

inline void preprocess_input_handle_comments_(Input input)
{
    ASSERT_INPUT_(input);

    for (unsigned long ind = 0; ind < input.text.nLines; ind++)
    {
        char *c_p = strchr(input.text.line_array[ind], COMMENT_SYMB);
        if (c_p)
        {
            *c_p = '\0';
        }
    }
}

inline void preprocess_input_tolower(Input input)
{
    ASSERT_INPUT_(input);

    for (size_t ind = 0; ind < input.file_buf.buf_size; ind++)
    {
        input.file_buf.buf[ind] = (char) tolower(input.file_buf.buf[ind]);
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!! TODO TOLOWER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void preprocess_input(Input input)
{
    ASSERT_INPUT_(input);

    preprocess_input_handle_comments_(input);

    preprocess_input_tolower(input);
}

inline void print_translation_error(unsigned long line, const char *str)
{
    fprintf(stderr, "ERROR on line %ld! The line is:\n<%s>\n",
                    line, str);
}

//! @brief Writes to the beginning of bin_arr the header, formed from constants in
//! assembler.h
inline void write_header_to_bin(char * bin_arr, size_t bin_final_len)
{
    *((int *) bin_arr) = *((const int *) SIGN);
    *((int *) bin_arr + 1) = VERSION;
    *((int *) bin_arr + 2) = (int) bin_final_len;
}

BinOut translate_to_binary(Input input)
{
    ASSERT_INPUT_(input);

    BinOut bin_out = {};

    // Примечание: считается, что всегда численное значение команды короче, чем ее словесное обозначение
    // (по количеству символов), а потому нам достаточно для транслированного текста того количества байтов,
    // которое было в изначальном
    char *bin_arr = (char *) calloc(input.file_buf.buf_size + HEADER_SIZE_IN_BYTES, sizeof(char));
    if (!bin_arr)
    {
        bin_out.err = ASM_ERROR_MEM_ALLOC;
        return bin_out;
    }
    size_t bin_arr_ind = HEADER_SIZE_IN_BYTES;

    for (unsigned long ind = 0; ind < input.text.nLines; ind++)
    {
        if ( input.text.line_array[ind][0] == '\0' ) continue;

        size_t cmd_end = 0;
        Command cmd = get_command(input.text.line_array[ind], &cmd_end);
        if (cmd == CMD_UNKNOWN)
        {
            print_translation_error(ind + 1, input.text.line_array[ind]);
            free(bin_arr); // ???
            bin_out.err = ASM_ERROR_UNKOWN_COMMAND;
            return bin_out;
        }

        bin_arr[bin_arr_ind] = (char) cmd;

        if (command_needs_arg[cmd])
        {
            CmdArg cmd_arg = get_arg(cmd, input.text.line_array[ind], cmd_end);
            if (cmd_arg.err)
            {
                bin_out.err = cmd_arg.err;
                return bin_out;
            }

            // перезаписываем тот же байт в bin_arr
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

    write_header_to_bin(bin_arr, bin_arr_ind);

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

Command get_command(const char *str, size_t *cmd_end_ptr)
{
    assert(str);
    assert(cmd_end_ptr);

    for (size_t cmd_ind = 1; cmd_ind < commands_list_len; cmd_ind++)
    {
        size_t str_ind = 0;

        while(1)
        {
            if ( str[str_ind] == commands_list[cmd_ind][str_ind] )
            {
                if ( str[str_ind] == '\0' )
                {
                    *cmd_end_ptr = str_ind;
                    return (Command) cmd_ind;
                }
                else
                {
                    str_ind++;
                }
            }
            else
            {
                if( str[str_ind] == ' ' && commands_list[cmd_ind][str_ind] == '\0' )
                {
                    *cmd_end_ptr = str_ind;
                    return (Command) cmd_ind;
                }
                else
                {
                    break;
                }
            }
        }
    }

    return CMD_UNKNOWN;
}

//! @brief Checks whether rgstr is a valid register's name or not.
//! Returns register's id or -1.
inline int check_reg_name(const char *rgstr)
{
    for ( size_t ind = 0; ind < num_of_registers; ind++ )
    {
        if ( strcmp(rgstr, registers_names[ind]) == 0 )
        {
            return (int) ind;
        }
    }
    return -1;
}

void print_asm_error_message(AssemblerError err)
{
    assert(err);

    switch (err)
    {
    case ASM_ERROR_GET_IN_OUT_FILES_NAMES:
        fprintf(stderr, "ASSEMBLER ERROR: Can't get input and output files' names!\n");
        break;
    case ASM_ERROR_READ_INPUT_FILE:
        fprintf(stderr, "ASSEMBLER ERROR: Can't read input file!\n");
        break;
    case ASM_ERROR_CANT_OPEN_OUTPUT_FILE:
        fprintf(stderr, "ASSEMBLER ERROR: Can't open output file!\n");
        break;
    case ASM_ERROR_MEM_ALLOC:
        fprintf(stderr, "ASSEMBLER ERROR: Can't allocate memory!\n");
        break;
    case ASM_ERROR_UNKOWN_COMMAND:
        fprintf(stderr, "ASSEMBLER ERROR: Unknown command in the input file!\n");
        break;
    case ASM_ERROR_CMD_ARG:
        fprintf(stderr, "ASSEMBLER ERROR: Invalid type of argument!\n");
        break;
    case ASM_ERROR_NO_ERROR:
    default:
        assert(0 && "Unreacheable default case in switch!");
        break;
    }
}

CmdArg get_arg(Command cmd, const char *line, size_t cmd_end)
{
    assert(line);

    CmdArg cmd_arg = {};

    int immediate_const = 0;
    char rgstr[register_name_len] = "";
    int reg_id = 0;

    if      ( command_needs_im_const_arg[(int) cmd]
            && sscanf(line + cmd_end, "%d", &immediate_const) == 1 )
    {
        cmd_arg.cmd_byte = ((char) cmd) | bit_immediate_const;
        cmd_arg.arg = (int) immediate_const;
        cmd_arg.arg_size = immediate_const_size_in_bytes;
        cmd_arg.err = ASM_ERROR_NO_ERROR;
    }
    else if ( command_needs_register_arg[(int) cmd]
            && sscanf(line + cmd_end, "%s", rgstr) == 1 && (reg_id = check_reg_name(rgstr)) != -1 )
    {
        cmd_arg.cmd_byte = ((char) cmd) | bit_register;
        cmd_arg.arg = (char) reg_id;
        cmd_arg.arg_size = register_id_size_in_bytes;
        cmd_arg.err = ASM_ERROR_NO_ERROR;
    }
    else
    {
        cmd_arg.err = ASM_ERROR_CMD_ARG;
    }

    return cmd_arg;
}

void free_struct_input(Input input)
{
    ASSERT_INPUT_(input);

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
