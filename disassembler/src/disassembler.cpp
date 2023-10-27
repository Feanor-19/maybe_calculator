#include <stdio.h>
#include <ctype.h>

#include "config_disasm.h"
#include "..\..\common\utils.h"

#include "disassembler.h"

int main(int argc, const char *argv[])
{
    Config cfg = get_config(argc, argv);
    if (cfg.error)
    {
        print_cfg_error_message(stderr, cfg.error);
        return DISASM_STATUS_ERROR_GET_IN_OUT_FILES_NAMES;
    }

    print_config(stdout, cfg);

    //----------------------------------------------------

    Binary binary = read_binary_file( cfg.input_file_name );
    CHECK_ERR_(binary.err);

    Output out = disassemble(binary);
    CHECK_ERR_(binary.err);

    DisasmStatus err = write_to_output_file(out, cfg.output_file_name);
    CHECK_ERR_(err);

    fprintf(stdout, "Disassembling is completed!\n");

    //----------------------------------------------------

    free_binary(&binary);

    return 0;
}

inline BIN_HEADER_FILE_SIZE_t get_binary_file_size_from_header_( int8_t* header)
{
    assert(header);

    return *( (BIN_HEADER_FILE_SIZE_t*) ( header + sizeof(BIN_HEADER_SIGN_t) + sizeof(BIN_HEADER_VERSION_t) ) );
}

Binary read_binary_file( const char *input_file_name )
{
    assert(input_file_name);

    Binary bin = {};

    FILE * f_inp = fopen(input_file_name, "rb");
    if (!f_inp)
    {
        bin.err = DISASM_STATUS_ERROR_READ_INPUT_FILE;
        return bin;
    }

    int8_t *input_file_header = (int8_t*) calloc( HEADER_SIZE_IN_BYTES, 1 );
    if (!input_file_header)
    {
        fclose(f_inp);
        bin.err = DISASM_STATUS_ERROR_MEM_ALLOC_ERROR;
        return bin;
    }

    if ( fread(input_file_header, 1, HEADER_SIZE_IN_BYTES, f_inp) != HEADER_SIZE_IN_BYTES )
    {
        fclose(f_inp);
        free(input_file_header);
        bin.err = DISASM_STATUS_ERROR_INP_FILE_HEADER_CORRUPTED;
        return bin;
    }

    BIN_HEADER_FILE_SIZE_t bin_file_size = get_binary_file_size_from_header_(input_file_header);
    if (bin_file_size <= HEADER_SIZE_IN_BYTES)
    {
        fclose(f_inp);
        free(input_file_header);
        bin.err = DISASM_STATUS_ERROR_INP_FILE_CORRUPTED;
        return bin;
    }

    int8_t *bin_arr = (int8_t*) realloc(input_file_header, (size_t) bin_file_size);

    if (!bin_arr)
    {
        fclose(f_inp);
        free(input_file_header);
        bin.err = DISASM_STATUS_ERROR_MEM_ALLOC_ERROR;
        return bin;
    }

    if ( fread(bin_arr + HEADER_SIZE_IN_BYTES, 1, (size_t) bin_file_size, f_inp)
        != (size_t) bin_file_size - HEADER_SIZE_IN_BYTES )
    {
        fclose(f_inp);
        free(input_file_header);
        bin.err = DISASM_STATUS_ERROR_INP_FILE_HEADER_CORRUPTED;
        return bin;
    }

    fclose(f_inp);

    bin.bin_arr = bin_arr;
    bin.binary_size = bin_file_size;
    bin.err = DISASM_STATUS_OK;

    return bin;
}

DisasmStatus realloc_output( Output* out_ptr )
{
    const size_t REALLOC_MULTUPLIER = 2;

    size_t ind = out_ptr->curr_char_ptr - out_ptr->text_buf;

    if ( out_ptr->curr_text_buf_size == 0 )
        out_ptr->curr_text_buf_size = DEFAULT_TEXT_BUF_SIZE;

    out_ptr->text_buf = (char *) realloc(out_ptr->text_buf, REALLOC_MULTUPLIER*out_ptr->curr_text_buf_size);
    if (!out_ptr->text_buf)
        return DISASM_STATUS_ERROR_MEM_ALLOC_ERROR;

    out_ptr->curr_text_buf_size *= REALLOC_MULTUPLIER;
    out_ptr->curr_char_ptr = out_ptr->text_buf + ind;


    return DISASM_STATUS_OK;
}

DisasmStatus write_chars_to_output( Output *out_ptr, char* char_arr, size_t nchars )
{
    assert(char_arr);
    assert(nchars > 0);

    size_t curr_out_ind = out_ptr->curr_char_ptr - out_ptr->text_buf;

    while ( curr_out_ind + nchars >= out_ptr->curr_text_buf_size )
    {
        realloc_output( out_ptr );
    }

    memcpy( out_ptr->curr_char_ptr, char_arr, nchars );
    out_ptr->curr_char_ptr += nchars;

    return DISASM_STATUS_OK;
}

inline void print_header_( Binary binary, Output *out_ptr )
{
    WRITE(out_ptr, "SIGN: ");
    for (size_t sign_ind = 0; sign_ind < sizeof(BIN_HEADER_SIGN_t); sign_ind++)
    {
        WRITE(out_ptr, "%c", *( (char*) (binary.bin_arr + sign_ind) ) );
    }
    WRITE(out_ptr, "\n");

    WRITE(out_ptr, "VERSION: %d\n", *( (BIN_HEADER_VERSION_t*) (binary.bin_arr + sizeof(BIN_HEADER_SIGN_t)) ) );
}

inline DisasmStatus print_curr_cmd_name( Binary *binary_ptr, Output *out_ptr)
{
    WRITE(out_ptr, "%s ", commands_list[ binary_ptr->bin_arr[binary_ptr->ip] ] );

    return DISASM_STATUS_OK;
}

inline uint8_t get_reg_id_( int8_t info_byte )
{
    uint8_t reg_id = 0;
    for ( uint8_t bit = BIT_REG_ID_START; bit <= BIT_REG_ID_END; bit++ )
    {
        if ( (uint8_t) info_byte & (1 << bit) )
        {
            reg_id = set_bit( reg_id, bit - BIT_REG_ID_START );
        }
    }
    return reg_id;
}

inline DisasmStatus print_curr_cmd_arg(Binary *binary_ptr, Output *out_ptr)
{
    int8_t info_byte = binary_ptr->bin_arr[binary_ptr->ip];

    binary_ptr->ip++;

    if ( test_bit( info_byte, BIT_IMMEDIATE_CONST ) )
    {
        immediate_const_t im_const_raw = *( (immediate_const_t*) (binary_ptr->bin_arr + binary_ptr->ip) );
        if ( test_bit(info_byte, BIT_MEMORY) )
        {
            //TODO - добавить спецификатор в common.h
            WRITE(out_ptr, "[%d]", (int) (im_const_raw / COMPUTATIONAL_MULTIPLIER) );
        }
        else
        {
            WRITE(out_ptr, "%lf", ((double) im_const_raw) / COMPUTATIONAL_MULTIPLIER );
        }
        binary_ptr->ip += sizeof(immediate_const_t);
    }
    else if ( test_bit( info_byte, BIT_REGISTER ) )
    {
        uint8_t reg_id = get_reg_id_( info_byte );
        if ( test_bit( info_byte, BIT_MEMORY ) )
        {
            WRITE(out_ptr, "[%s]", registers_names[reg_id]);
        }
        else
        {
            WRITE(out_ptr, "%s", registers_names[reg_id]);
        }

    }
    else if ( test_bit( info_byte, BIT_CS_OFFSET ) )
    {
        cs_offset_t offset = *( (cs_offset_t*) ( binary_ptr->bin_arr + binary_ptr->ip ) );
        WRITE(out_ptr, "%u", offset);
        binary_ptr->ip += sizeof(cs_offset_t);
    }

    WRITE(out_ptr, "\n");

    return DISASM_STATUS_OK;
}

Output disassemble( Binary binary )
{
    Output out = {};

    print_header_(binary, &out);

    binary.ip = HEADER_SIZE_IN_BYTES;
    while (binary.ip < binary.binary_size)
    {
        WRITE(&out, "%d\t\t\t", (int) (binary.ip - HEADER_SIZE_IN_BYTES) );

        DisasmStatus err = print_curr_cmd_name( &binary, &out );
        if (err)
        {
            out.err = err;
            return out;
        }

        binary.ip++;

        err = print_curr_cmd_arg( &binary, &out );
        if (err)
        {
            out.err = err;
            return out;
        }
    }

    return out;
}

DisasmStatus write_to_output_file( Output out, const char *output_file_name )
{
    assert(output_file_name);

    FILE* f_out = fopen(output_file_name, "w");
    if (!f_out)
    {
        return DISASM_STATUS_ERROR_OPEN_OUTPUT_FILE;
    }

    fwrite(out.text_buf, sizeof(char), out.curr_char_ptr - out.text_buf, f_out);

    fclose(f_out);

    return DISASM_STATUS_OK;
}

void free_binary( Binary * bin_ptr )
{
    if (bin_ptr->bin_arr)
        free(bin_ptr->bin_arr);

    bin_ptr->bin_arr = NULL;
    bin_ptr->binary_size = 0;
}

void print_disasm_error_message( DisasmStatus err )
{
    assert(err);

    fprintf(stderr, "DISASSEMBLER ERROR: <%s>!\n", disassembler_error_messages[(int) err]);
}
