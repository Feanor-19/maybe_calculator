#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#include "assembler.h"

struct Fixup
{
    size_t cs_offset_arg_location;          //< Номер байта с начала _бинарного файла_ в который надо записать смещение метки
    char label_name[LABEL_NAME_MAX_LEN];    //< Имя метки
};

typedef Fixup Elem_t;
inline void print_elem_t(FILE *stream, Elem_t val)
{
    fprintf(stream, "{<%lld> : <%s>}", val.cs_offset_arg_location, val.label_name);
}
#define STACK_DO_DUMP
#define STACK_USE_POISON
#define STACK_USE_PROTECTION_CANARY
#define STACK_USE_PROTECTION_HASH
#include "..\..\common\stack.h"

#include "config_asm.h"
#include "..\..\common\utils.h"
#include "..\..\common\mystring.h"

int main(int argc, const char *argv[])
{
    Config cfg = get_config(argc, argv);
    if (cfg.error)
    {
        print_cfg_error_message(stderr, cfg.error);
        return ASM_STATUS_ERROR_GET_IN_OUT_FILES_NAMES;
    }

    print_config(stdout, cfg);



    Input input = read_input_file(cfg.input_file_name);
    CHECK_ERR_(input.err);

    preprocess_input(input);

    BinOut bin_out = translate_to_binary(input);
    CHECK_ERR_(bin_out.err);

    AssemblerStatus err = write_bin_to_output(bin_out, cfg.output_file_name);
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
        input.err = ASM_STATUS_ERROR_READ_INPUT_FILE;
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

inline void preprocess_input_toupper(Input input)
{
    ASSERT_INPUT_(input);

    for (size_t ind = 0; ind < input.file_buf.buf_size; ind++)
    {
        input.file_buf.buf[ind] = (char) toupper(input.file_buf.buf[ind]);
    }
}

//! @brief Moves pointers in input.text.line_array so that initial spaces are skipped
inline void preprocess_input_skip_initial_spaces(Input input)
{
    for (unsigned long ind = 0; ind < input.text.nLines; ind++)
    {
        char *ptr = input.text.line_array[ind];
        while ( *ptr == ' ' && *ptr != '\0')
            ptr++;
        input.text.line_array[ind] = ptr;
    }
}

void preprocess_input(Input input)
{
    ASSERT_INPUT_(input);

    preprocess_input_handle_comments_(input);

    preprocess_input_toupper(input);

    preprocess_input_skip_initial_spaces(input);
}

inline void print_translation_error(unsigned long line, const char *str)
{
    fprintf(stderr, "ERROR on line %ld! The line is:\n<%s>\n",
                    line, str);
}

//! @brief Writes to the beginning of bin_arr the header, formed from constants in
//! assembler.h
inline void write_header_to_bin(int8_t * bin_arr, size_t bin_final_len)
{
    *((BIN_HEADER_SIGN_t *) bin_arr) = SIGN;
    bin_arr += sizeof(BIN_HEADER_SIGN_t);

    *((BIN_HEADER_VERSION_t *) bin_arr) = VERSION;
    bin_arr += sizeof(BIN_HEADER_VERSION_t);

    *((BIN_HEADER_FILE_SIZE_t *) bin_arr) = (BIN_HEADER_FILE_SIZE_t) bin_final_len;
}

inline int can_cmd_have_arg(Command cmd)
{
    return command_needs_im_const_arg[(int) cmd]
        || command_needs_register_arg[(int) cmd]
        ||   command_needs_memory_arg[(int) cmd]
        ||    command_needs_label_arg[(int) cmd];
}

inline AssemblerStatus handle_cmd_arg(  int8_t *bin_arr,
                                        size_t *bin_arr_ind_ptr,
                                        CmdArg cmd_arg,
                                        Stack *fixup_stk_ptr )
{

    bin_arr[(*bin_arr_ind_ptr)++] = cmd_arg.info_byte;
    if ( test_bit(cmd_arg.info_byte, BIT_IMMEDIATE_CONST) )
    {
        *((immediate_const_t *) (bin_arr + (*bin_arr_ind_ptr))) = cmd_arg.im_const;
        (*bin_arr_ind_ptr) += sizeof(immediate_const_t);
    }
    else if ( test_bit(cmd_arg.info_byte, BIT_CS_OFFSET) )
    {
        Fixup tmp = {};
        tmp.cs_offset_arg_location = (*bin_arr_ind_ptr);
        strncpy(tmp.label_name, cmd_arg.label_name, LABEL_NAME_MAX_LEN);
        stack_push(fixup_stk_ptr, tmp);

        *((cs_offset_t *) (bin_arr + (*bin_arr_ind_ptr))) = CS_OFFSET_POISON_VALUE;
        (*bin_arr_ind_ptr) += sizeof(cs_offset_t);
    }
    return ASM_STATUS_OK;
}

inline Label* find_label( Label labels[], const char *label_name )
{
    for (size_t ind = 0; ind < MAX_LABELS_COUNT; ind++)
    {
        if ( strcmp(labels[ind].name, label_name) == 0 )
            return labels + ind;
    }
    return NULL;
}

inline int check_last_char(const char *str, char c)
{
    return str[strlen(str) - 1] == c;
}

//! @brief Checks if str contains a valid label (without spaces in it and ends with ':')
// and remembers label's location and name.
inline AssemblerStatus check_if_label_and_handle( const char *str, size_t bin_arr_ind, Label labels[] )
{
    char label[LABEL_NAME_MAX_LEN] = "---";
    if ( sscanf(str, "%s", label) != 1 || !check_last_char(label, ':'))
        return ASM_STATUS_OK; // not a label

    static size_t labels_ind = 0;

    if ( find_label(labels, label) )
        return ASM_STATUS_ERROR_LABEL_REDEFINED;

    Label tmp = {};
    tmp.bin_arr_ind = bin_arr_ind;
    strncpy(tmp.name, label, strlen(label) - 1);

    labels[labels_ind++] = tmp;
    return ASM_STATUS_CURR_LINE_IS_A_LABEL;
}

inline AssemblerStatus handle_fixup(int8_t* bin_arr, Stack *fixup_stk_ptr, Label labels[])
{
    Fixup fixup = {};
    while ( stack_pop( fixup_stk_ptr, &fixup ) != STACK_ERROR_NOTHING_TO_POP )
    {
        Label *lbl_ptr = find_label(labels, fixup.label_name );
        if (!lbl_ptr)
            return ASM_STATUS_ERROR_UNDEFINED_LABEL;

        *((cs_offset_t *) (bin_arr + fixup.cs_offset_arg_location)) = (cs_offset_t) (lbl_ptr->bin_arr_ind - HEADER_SIZE_IN_BYTES);
    }

    return ASM_STATUS_OK;
}

BinOut translate_to_binary(Input input)
{
    ASSERT_INPUT_(input);

    BinOut bin_out = {};

    // Примечание: считается, что всегда численное значение команды короче, чем ее словесное обозначение
    // (по количеству символов), а потому нам достаточно для транслированного текста того количества байтов,
    // которое было в изначальном
    const size_t START_BIN_ARR_SIZE = input.file_buf.buf_size + HEADER_SIZE_IN_BYTES;
    int8_t *bin_arr = (int8_t *) calloc(START_BIN_ARR_SIZE, sizeof(char));
    if (!bin_arr)
    {
        bin_out.err = ASM_STATUS_ERROR_MEM_ALLOC;
        return bin_out;
    }
    size_t bin_arr_ind = HEADER_SIZE_IN_BYTES;

    Stack fixup_stk = {};
    stack_ctor(&fixup_stk);

    Label labels[MAX_LABELS_COUNT] = {};

    for (unsigned long ind = 0; ind < input.text.nLines; ind++)
    {
        if ( is_str_empty(input.text.line_array[ind]) ) continue;

        AssemblerStatus label_status = check_if_label_and_handle( input.text.line_array[ind], bin_arr_ind, labels);
        if (label_status == ASM_STATUS_CURR_LINE_IS_A_LABEL)
            continue;
        else if (label_status == ASM_STATUS_ERROR_LABEL_REDEFINED)
        {
            print_translation_error(ind + 1, input.text.line_array[ind]);
            free(bin_arr);
            bin_out.err = ASM_STATUS_ERROR_LABEL_REDEFINED;
            return bin_out;
        }

        size_t cmd_end = 0;
        Command cmd = get_command(input.text.line_array[ind], &cmd_end);
        if (cmd == CMD_UNKNOWN)
        {
            print_translation_error(ind + 1, input.text.line_array[ind]);
            free(bin_arr);
            bin_out.err = ASM_STATUS_ERROR_UNKNOWN_COMMAND;
            return bin_out;
        }

        bin_arr[bin_arr_ind++] = (int8_t) cmd;
        CmdArg cmd_arg = {};
        if ( can_cmd_have_arg(cmd) )
        {
            cmd_arg = get_arg(cmd, input.text.line_array[ind] + cmd_end);
            if (cmd_arg.err)
            {
                bin_out.err = cmd_arg.err;
                return bin_out;
            }
        }
        else
        {
            cmd_arg.info_byte = 0x00;
            cmd_arg.err = ASM_STATUS_OK;
        }

        handle_cmd_arg(bin_arr, &bin_arr_ind, cmd_arg, &fixup_stk);
    }

    AssemblerStatus error = handle_fixup(bin_arr, &fixup_stk, labels);
    if (error)
    {
        free(bin_arr);
        bin_out.err = error;
        return bin_out;
    }

    write_header_to_bin(bin_arr, bin_arr_ind);

    bin_out.bin_arr = bin_arr;
    bin_out.bin_arr_len = bin_arr_ind;
    bin_out.err = ASM_STATUS_OK;

    stack_dtor(&fixup_stk);

    return bin_out;
}

AssemblerStatus write_bin_to_output(BinOut bin_out, const char *output_file_name)
{
    FILE* out = fopen(output_file_name, "wb");
    if (!out)
    {
        fclose(out);
        return ASM_STATUS_ERROR_CANT_OPEN_OUTPUT_FILE;
    }

    fwrite(bin_out.bin_arr, sizeof(char), bin_out.bin_arr_len, out);

    fclose(out);

    return ASM_STATUS_OK;
}

inline size_t find_cmd_end( const char *str)
{
    char *cmd_end_char_ptr = strchr(str, ' ');
    if ( !cmd_end_char_ptr )
        return strchr(str, '\0') - str;
    else
        return cmd_end_char_ptr - str;
}

Command get_command(char *str, size_t *cmd_end_ptr)
{
    assert(str);
    assert(cmd_end_ptr);

    *cmd_end_ptr = find_cmd_end(str);
    int space_was_changed_to_zero = 0;
    if ( str[*cmd_end_ptr] == ' ' )
    {
        str[*cmd_end_ptr] = '\0';
        space_was_changed_to_zero = 1;
    }

    for (size_t cmd_ind = 1; cmd_ind < commands_list_len; cmd_ind++)
    {
        if ( strcmp(str, commands_list[cmd_ind] ) == 0 )
        {
            if ( space_was_changed_to_zero )
                str[*cmd_end_ptr] = ' ';

            return (Command) cmd_ind;
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

inline uint8_t write_reg_to_info_byte( uint8_t info_byte, int reg_id )
{
    for ( uint8_t bit = BIT_REG_ID_START; bit <= BIT_REG_ID_END; bit++ )
    {
        if ( test_bit((uint8_t) reg_id, bit - BIT_REG_ID_START) )
            info_byte = set_bit( info_byte, bit );
    }
    return info_byte;
}

CmdArg get_arg(Command cmd, const char *arg)
{
    assert(arg);

    CmdArg cmd_arg = {};

    extern_num_t immediate_const_raw = 0;
    char rgstr[register_name_len + 1] = "";
    int reg_id = 0;

    if  ( command_needs_im_const_arg[(int) cmd]
        && sscanf(arg, "%lf", &immediate_const_raw) == 1 )
    {
        if ( (int) (immediate_const_raw) >= INT_MAX / COMPUTATIONAL_MULTIPLIER )
        {
            cmd_arg.err = ASM_STATUS_ERROR_CMD_ARG_TOO_BIG;
            return cmd_arg;
        }

        cmd_arg.info_byte = set_bit( cmd_arg.info_byte, BIT_IMMEDIATE_CONST );
        cmd_arg.im_const = (immediate_const_t) (immediate_const_raw * COMPUTATIONAL_MULTIPLIER);
        cmd_arg.err = ASM_STATUS_OK;

        return cmd_arg;
    }
    else if ( command_needs_register_arg[(int) cmd]
            && sscanf(arg, "%s", rgstr) == 1 && (reg_id = check_reg_name(rgstr)) != -1 )
    {
        cmd_arg.info_byte = set_bit( cmd_arg.info_byte, BIT_REGISTER ) ;
        cmd_arg.info_byte = write_reg_to_info_byte( cmd_arg.info_byte, reg_id );
        cmd_arg.err = ASM_STATUS_OK;

        return cmd_arg;
    }
    else if ( command_needs_memory_arg[(int) cmd] )
    {
        if ( sscanf(arg, " [ %lf ] ", &immediate_const_raw) == 1 )
        {
            cmd_arg.info_byte = set_bit( cmd_arg.info_byte, BIT_MEMORY );
            cmd_arg.info_byte = set_bit( cmd_arg.info_byte, BIT_IMMEDIATE_CONST );
            cmd_arg.im_const = (immediate_const_t) (immediate_const_raw * COMPUTATIONAL_MULTIPLIER);
            cmd_arg.err = ASM_STATUS_OK;

            return cmd_arg;
        }
        else if ( sscanf(arg, " [ %[^]] ] ", rgstr) == 1 && (reg_id = check_reg_name(rgstr)) != -1 )
        {
            cmd_arg.info_byte = set_bit( cmd_arg.info_byte, BIT_MEMORY );
            cmd_arg.info_byte = set_bit( cmd_arg.info_byte, BIT_REGISTER );
            cmd_arg.info_byte = write_reg_to_info_byte( cmd_arg.info_byte, reg_id);
            cmd_arg.err = ASM_STATUS_OK;

            return cmd_arg;
        }
    }
    else if ( command_needs_label_arg[(int) cmd]
            && sscanf(arg, "%s", cmd_arg.label_name) == 1 )
    {
        cmd_arg.info_byte = set_bit(cmd_arg.info_byte, BIT_CS_OFFSET);
        cmd_arg.err = ASM_STATUS_OK;

        return cmd_arg;
    }

    cmd_arg.err = ASM_STATUS_ERROR_CMD_ARG;

    return cmd_arg;
}

void print_asm_error_message(AssemblerStatus err)
{
    assert(err);

    fprintf(stderr, "ASSEMBLER ERROR: <%s>!\n", assembler_error_messages[(int) err]);
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
