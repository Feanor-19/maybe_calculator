#include <stdio.h>

#include "config_spu.h"

typedef int Elem_t;
inline void print_elem_t(FILE *stream, Elem_t val) {fprintf(stream, "%d", val);}
#define STACK_DO_DUMP
#define STACK_USE_POISON
#define STACK_DUMP_ON_INVALID_POP
#define STACK_USE_PROTECTION_CANARY
#define STACK_USE_PROTECTION_HASH
#include "stack.h"

#include "spu.h"

int main(int argc, const char *argv[])
{
    Config cfg = get_config(argc, argv);
    if (cfg.error)
    {
        print_cfg_error_message(stderr, cfg.error);
        return SPU_STATUS_ERROR_GET_IN_OUT_FILES_NAMES;
    }

    print_config(stdout, cfg);

    //----------------------

    SPU spu = {};
    SPUStatus spu_err = SPU_STATUS_OK;

    spu_err = SPU_ctor(&spu, cfg);
    PRINT_IF_SPU_ERROR_(spu_err);

    double prog_res = -1;
    run_program(&spu, &prog_res);
    printf("Result of the program: <%lf>", prog_res);

    SPU_dtor(&spu);

    return 0;
}

//! @brief Checks given header and writes size of code in bytes to *cs_size.
inline SPUStatus check_header_(char *input_file_header, size_t *cs_size)
{
    assert(input_file_header);
    assert(cs_size);

    if ( *((BIN_HEADER_SIGN_t *) input_file_header) != SIGN)
        return SPU_STATUS_ERROR_HEADER_WRONG_SIGN;

    input_file_header += sizeof(BIN_HEADER_SIGN_t);

    if ( *((BIN_HEADER_VERSION_t *) input_file_header) != VERSION )
        return SPU_STATUS_ERROR_INPUT_FILE_WRONG_VERSION;

    input_file_header += sizeof(BIN_HEADER_VERSION_t);

    *cs_size = (size_t) *((BIN_HEADER_FILE_SIZE_t *) input_file_header) - HEADER_SIZE_IN_BYTES;

    return SPU_STATUS_OK;
}

//! @brief Reads input file, checks it's header, allocates memory for asm prog and loads
//! it into spu.
inline SPUStatus load_prog_into_spu_(SPU *spu_ptr, const char *input_file_name)
{
    // НЕ SPU_CHECK, А ASSERT, ПОТОМУ ЧТО ЭТА ФУНКЦИЯ
    // ВЫЗЫВАЕТСЯ В КОНСТРУКТОРЕ, КОГДА SPU ЕЩЁ НЕ ГОТОВ
    assert(spu_ptr);
    assert(input_file_name);

    FILE * f_inp = fopen(input_file_name, "rb");
    if (!f_inp) return SPU_STATUS_ERROR_READ_INPUT_FILE;

    char *input_file_header = (char *) calloc( HEADER_SIZE_IN_BYTES, 1 );
    if (!input_file_header)
    {
        fclose(f_inp);
        return SPU_STATUS_ERROR_MEM_ALLOC_ERROR;
    }

    if ( fread(input_file_header, 1, HEADER_SIZE_IN_BYTES, f_inp) != HEADER_SIZE_IN_BYTES )
    {
        fclose(f_inp);
        free(input_file_header);
        return SPU_STATUS_ERROR_INP_FILE_HEADER_CORRUPTED;
    }

    size_t cs_size = 0;
    SPUStatus err = check_header_(input_file_header, &cs_size);
    if (cs_size == 0 || err)
    {
        fclose(f_inp);
        free(input_file_header);
        if (err) return err;
        if (cs_size == 0) return SPU_STATUS_ERROR_ZERO_BYTES_OF_CODE;
    }

    char * cs_ptr = (char *) realloc(input_file_header, cs_size);
    if (!cs_ptr)
    {
        fclose(f_inp);
        free(input_file_header);
        return SPU_STATUS_ERROR_MEM_ALLOC_ERROR;
    }

    if ( fread(cs_ptr, 1, cs_size, f_inp) != cs_size )
    {
        fclose(f_inp);
        free(input_file_header);
        return SPU_STATUS_ERROR_INP_FILE_HEADER_CORRUPTED;
    }

    fclose(f_inp);

    spu_ptr->cs = cs_ptr;
    spu_ptr->cs_size = cs_size;

    return SPU_STATUS_OK;
}

SPUStatus SPU_ctor(SPU* spu_ptr, Config cfg)
{
    if (!spu_ptr) return SPU_STATUS_ERROR_NULL_SPU_PTR;
    if (cfg.error) return SPU_STATUS_ERROR_CONFIG_HAS_ERROR;

    STACK_FUNC_WRAP( stack_ctor(&(spu_ptr->stk)) );

    for (size_t ind = 0; ind < num_of_registers; ind++)
    {
        spu_ptr->registers[ind] = 0;
    }

    SPUStatus err = load_prog_into_spu_(spu_ptr, cfg.input_file_name);
    if (err) return err;

    return SPU_STATUS_OK;
}

SPUStatus SPU_dtor(SPU* spu_ptr)
{
    if (!spu_ptr) return SPU_STATUS_ERROR_NULL_SPU_PTR;

    STACK_FUNC_WRAP( stack_dtor( &(spu_ptr->stk) ) );

    for (size_t ind = 0; ind < num_of_registers; ind++)
    {
        spu_ptr->registers[ind] = (int) 0xFFFFFFFF;
    }

    if (spu_ptr->cs) free(spu_ptr->cs);
    spu_ptr->cs_size = (size_t) -1;

    return SPU_STATUS_OK;
}

int SPU_verificator(SPU* spu_ptr)
{
    int error = 0;

    if ( !spu_ptr )
        error |= SPU_VERIFY_NULL_PNT;

    if ( spu_ptr && stack_verify(&spu_ptr->stk) )
        error |= SPU_VERIFY_STACK_VERIFY_ERROR;

    if ( spu_ptr && !spu_ptr->cs )
        error |= SPU_VERIFY_CS_PTR_NULL;

    if ( spu_ptr && spu_ptr->cs_size > ((size_t) -10) )
        error |= SPU_VERIFY_CS_SIZE_TOO_BIG;

    if ( spu_ptr && spu_ptr->ip >= spu_ptr->cs_size )
        error |= SPU_VERIFY_IP_VALUE_INVALID;

    return error;
}

void print_spu_verify_res_(int verify_res)
{
    fprintf(stderr, "SPU verification result: <%d>\n", verify_res);
    for (size_t ind = 0; ind < NUMBER_OF_SPU_VERIFY_RES_FLAGS; ind++)
    {
        if (verify_res & ( 1 << (ind + SPU_FIRST_VERIFY_RES_ID) ))
        {
            printf("----> %s\n", spu_verification_messages[ind]);
        }
    }
}

inline void print_spu_registers_(SPU *spu_ptr)
{
    assert(spu_ptr);

    fprintf(stderr, "SPU REGISTERS:\n");
    for (size_t ind = 0; ind < num_of_registers; ind++)
    {
        fprintf(stderr, "\t[%s] : <%d>\n", registers_names[ind], spu_ptr->registers[ind]);
    }
}

inline void print_header_bytes_(const BIN_HEADER_SIGN_t sign,
                                const BIN_HEADER_VERSION_t version,
                                const BIN_HEADER_FILE_SIZE_t binary_size_in_bytes)
{
    for (size_t ind = 0; ind < sizeof(BIN_HEADER_SIGN_t); ind++)
    {
        fprintf(stderr, "%02X ", ( (const char*) &sign )[ind] );
    }

    for (size_t ind = 0; ind < sizeof(BIN_HEADER_VERSION_t); ind++)
    {
        fprintf(stderr, "%02X ", ( (const char*) &version )[ind] );
    }

    for (size_t ind = 0; ind < sizeof(BIN_HEADER_FILE_SIZE_t); ind++)
    {
        fprintf(stderr, "%02X ", ( (const char*) &binary_size_in_bytes )[ind] );
    }
}

inline void put_n_chars(FILE *stream, size_t n, char c)
{
    while (n--)
    {
        putc(c, stream);
    }
}

inline void print_spu_header_and_cs_(SPU *spu_ptr)
{
    assert(spu_ptr);

    fprintf(stderr, "HEADER + CODE SEGMENT:\n{\n");

    print_header_bytes_(SIGN, VERSION, spu_ptr->cs_size + HEADER_SIZE_IN_BYTES);

    const size_t row_width = 16;

    size_t rows_count = (spu_ptr->cs_size + HEADER_SIZE_IN_BYTES) / row_width + ( spu_ptr->cs_size % row_width != 0 );
    size_t ind = 0;
    for (size_t row = 0; row < rows_count; row++ )
    {
        const size_t col_of_ip_on_this_row_default_value = row_width + 1;
        size_t col_of_ip_on_this_row = col_of_ip_on_this_row_default_value;
        for (size_t col = 0; col < row_width; col++)
        {
            if (row == 0 && col == 0)
            {
                // because header is already printed
                col = HEADER_SIZE_IN_BYTES;
            }

            ind = row*row_width + col - HEADER_SIZE_IN_BYTES;

            if ( ind >= spu_ptr->cs_size )
                break;

            fprintf(stderr, "%02X ", (char) spu_ptr->cs[ind]);
            if (ind == spu_ptr->ip)
                col_of_ip_on_this_row = col;
        }
        putc('\n', stderr);
        if (col_of_ip_on_this_row != col_of_ip_on_this_row_default_value)
        {
            put_n_chars(stderr, 3*col_of_ip_on_this_row, ' ');
            putc('^', stderr);
        }
        putc('\n', stderr);
    }
    fprintf(stderr, "}\n");
}

void SPU_dump_( SPU* spu_ptr,
                    const char* file_name,
                    int line,
                    const char* func_name,
                    int verify_res)
{
    fprintf(stderr, "====================== SPU DUMP ======================\n");
    fprintf(stderr, "%s(%d), in function %s.\n", file_name, line, func_name);

    print_spu_verify_res_( verify_res != 0 ? verify_res : SPU_verificator(spu_ptr) );
    print_spu_registers_(spu_ptr);

    fprintf(stderr, "ip: <%llu>\n", spu_ptr->ip);

    if (spu_ptr->cs)
    {
        print_spu_header_and_cs_(spu_ptr);
    }

    fprintf(stderr, "STACK DUMP:\n");
    STACK_DUMP(&spu_ptr->stk, stack_verify(&spu_ptr->stk));
    fprintf(stderr, "STACK DUMP END!\n");

    fprintf(stderr, "====================== DUMP END ======================\n");
}

void print_spu_error(SPUStatus err)
{
    assert(err);
    fprintf(stderr, "SPU ERROR: <%s>\n", spu_status_messages[(int) err]);
}

SPUStatus run_program(SPU *spu_ptr, double *prog_res)
{
    SPU_CHECK(spu_ptr);
    assert(prog_res);

    while (1) // типа процессор не остановится пока не встретит hlt
    {
        SPUStatus exec_res = exec_curr_cmd_(spu_ptr, prog_res);
        if (exec_res != SPU_STATUS_OK && exec_res != SPU_STATUS_HLT)
        {
            print_spu_error(exec_res);
            SPU_DUMP(spu_ptr, 0);
            return exec_res;
        }
        else if ( exec_res == SPU_STATUS_HLT )
            return SPU_STATUS_OK;
    }
}

inline SPUStatus exec_cmd_push(SPU *spu_ptr)
{
    SPU_CHECK(spu_ptr);

    if ( spu_ptr->cs[spu_ptr->ip] & BIT_IMMEDIATE_CONST )
    {
        spu_ptr->ip++;
        int im_const = *((int *) (spu_ptr->cs + spu_ptr->ip));
        STACK_FUNC_WRAP( stack_push( &spu_ptr->stk, im_const) );
        spu_ptr->ip += sizeof(int);
    }
    else if ( spu_ptr->cs[spu_ptr->ip] & BIT_REGISTER )
    {
        spu_ptr->ip++;
        char reg = spu_ptr->cs[spu_ptr->ip];
        STACK_FUNC_WRAP( stack_push( &spu_ptr->stk, spu_ptr->registers[(int) reg]) );
        spu_ptr->ip += sizeof(char);
    }
    else
    {
        return SPU_STATUS_ERROR_UNKNOWN_CMD;
    }

    return SPU_STATUS_OK;
}

inline SPUStatus exec_cmd_pop(SPU *spu_ptr)
{
    SPU_CHECK(spu_ptr);

    if ( spu_ptr->cs[spu_ptr->ip] & BIT_REGISTER )
    {
        spu_ptr->ip++;
        char reg = spu_ptr->cs[spu_ptr->ip];
        STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &(spu_ptr->registers[(int) reg]) ) );
        spu_ptr->ip += sizeof(char);
    }
    else
    {
        return SPU_STATUS_ERROR_UNKNOWN_CMD;
    }

    return SPU_STATUS_OK;
}

inline SPUStatus exec_cmd_add(SPU *spu_ptr)
{
    SPU_CHECK(spu_ptr);

    int a = 0, b = 0;
    STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &b ) );
    STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &a ) );
    STACK_FUNC_WRAP( stack_push( &spu_ptr->stk, a + b ) );

    spu_ptr->ip++;

    return SPU_STATUS_OK;
}

inline SPUStatus exec_cmd_sub(SPU *spu_ptr)
{
    SPU_CHECK(spu_ptr);

    int a = 0, b = 0;
    STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &b ) );
    STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &a ) );
    STACK_FUNC_WRAP( stack_push( &spu_ptr->stk, a - b ) );

    spu_ptr->ip++;

    return SPU_STATUS_OK;
}

inline SPUStatus exec_cmd_mul(SPU *spu_ptr)
{
    SPU_CHECK(spu_ptr);

    int a = 0, b = 0;
    STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &b ) );
    STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &a ) );
    STACK_FUNC_WRAP( stack_push( &spu_ptr->stk, (a * b) / COMPUTATIONAL_MULTIPLIER ) );

    spu_ptr->ip++;

    return SPU_STATUS_OK;
}

inline SPUStatus exec_cmd_div(SPU *spu_ptr)
{
    SPU_CHECK(spu_ptr);

    int a = 0, b = 0;
    STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &b ) );
    STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &a ) );
    STACK_FUNC_WRAP( stack_push( &spu_ptr->stk, (a / b)*COMPUTATIONAL_MULTIPLIER ) );

    spu_ptr->ip++;

    return SPU_STATUS_OK;
}


inline SPUStatus exec_cmd_in(SPU *spu_ptr)
{
    SPU_CHECK(spu_ptr);

    double in = 0;

    fprintf(stdout, "Please enter 'in':\n");
    if ( fscanf(stdin, "%lf", &in) != 1 )
        return SPU_STATUS_ERROR_WRONG_IN;

    STACK_FUNC_WRAP( stack_push( &spu_ptr->stk, (int) (in * COMPUTATIONAL_MULTIPLIER) ) );

    spu_ptr->ip++;

    return SPU_STATUS_OK;
}


inline SPUStatus exec_cmd_out(SPU *spu_ptr, double *prog_res)
{
    SPU_CHECK(spu_ptr);
    assert(prog_res);

    int buf = 0;
    STACK_FUNC_WRAP( stack_pop( &spu_ptr->stk, &buf ) );
    *prog_res = ( (double) buf ) / COMPUTATIONAL_MULTIPLIER;

    spu_ptr->ip++;

    return SPU_STATUS_OK;
}

inline

#define DEF_CMD(name, id, is_im_const, is_reg, is_mem, ...)     \
    case CMD_##name:                                            \
        __VA_ARGS__                                             \
        break;

SPUStatus exec_curr_cmd_(SPU *spu_ptr, double *prog_res)
{
    SPU_CHECK(spu_ptr);
    assert(prog_res);

    switch ( spu_ptr->cs[spu_ptr->ip] )
    {
    #include "../../common/commands.h"
    default:
        return SPU_STATUS_ERROR_UNKNOWN_CMD;
        break;
    }

    return SPU_STATUS_OK;
}
#undef DEF_CMD
