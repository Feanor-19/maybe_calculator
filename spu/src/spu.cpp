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
        return SPU_ERROR_GET_IN_OUT_FILES_NAMES;
    }

    print_config(stdout, cfg);

    //----------------------

    SPU spu = {};
    SPUError spu_err = SPU_ERROR_NO_ERROR;

    spu_err = SPU_ctor(&spu, cfg);
    PRINT_IF_SPU_ERROR_(spu_err);

    SPU_DUMP(&spu, 0);


    SPU_dtor(&spu);

    return 0;
}

//! @brief Checks given header and writes size of code in bytes to *cs_size.
inline SPUError check_header_(char *input_file_header, size_t *cs_size)
{
    assert(input_file_header);
    assert(cs_size);

    if ( *((int *) input_file_header) != *((const int *) SIGN))
        return SPU_ERROR_HEADER_WRONG_SIGN;

    if ( *((int *) input_file_header + 1) != VERSION )
        return SPU_ERROR_INPUT_FILE_WRONG_VERSION;

    *cs_size = (size_t) *((int *) input_file_header + 2) - HEADER_SIZE_IN_BYTES;

    return SPU_ERROR_NO_ERROR;
}

//! @brief Reads input file, checks it's header, allocates memory for asm prog and loads
//! it into spu.
inline SPUError load_prog_into_spu_(SPU *spu_ptr, const char *input_file_name)
{
    // НЕ SPU_CHECK, А ASSERT, ПОТОМУ ЧТО ЭТА ФУНКЦИЯ
    // ВЫЗЫВАЕТСЯ В КОНСТРУКТОРЕ, КОГДА SPU ЕЩЁ НЕ ГОТОВ
    assert(spu_ptr);
    assert(input_file_name);

    FILE * f_inp = fopen(input_file_name, "rb");
    if (!f_inp) return SPU_ERROR_READ_INPUT_FILE;

    char *input_file_header = (char *) calloc( HEADER_SIZE_IN_BYTES, 1 );
    if (!input_file_header)
    {
        fclose(f_inp);
        return SPU_ERROR_MEM_ALLOC_ERROR;
    }

    if ( fread(input_file_header, 1, HEADER_SIZE_IN_BYTES, f_inp) != HEADER_SIZE_IN_BYTES )
    {
        fclose(f_inp);
        free(input_file_header);
        return SPU_ERROR_INP_FILE_HEADER_CORRUPTED;
    }

    size_t cs_size = 0;
    SPUError err = check_header_(input_file_header, &cs_size);
    if (cs_size == 0 || err)
    {
        fclose(f_inp);
        free(input_file_header);
        if (cs_size == 0) return SPU_ERROR_ZERO_BYTES_OF_CODE;
        if (err) return err;
    }

    char * cs_ptr = (char *) realloc(input_file_header, cs_size);
    if (!cs_ptr)
    {
        fclose(f_inp);
        free(input_file_header);
        return SPU_ERROR_MEM_ALLOC_ERROR;
    }

    if ( fread(cs_ptr, 1, cs_size, f_inp) != cs_size )
    {
        fclose(f_inp);
        free(input_file_header);
        return SPU_ERROR_INP_FILE_HEADER_CORRUPTED;
    }

    fclose(f_inp);

    spu_ptr->cs = cs_ptr;
    spu_ptr->cs_size = cs_size;

    return SPU_ERROR_NO_ERROR;
}

SPUError SPU_ctor(SPU* spu_ptr, Config cfg)
{
    if (!spu_ptr) return SPU_ERROR_NULL_SPU_PTR;
    if (cfg.error) return SPU_ERROR_CONFIG_HAS_ERROR;

    STACK_FUNC_WRAP( stack_ctor(&(spu_ptr->stk)) );

    for (size_t ind = 0; ind < num_of_registers; ind++)
    {
        spu_ptr->registers[ind] = 0;
    }

    SPUError err = load_prog_into_spu_(spu_ptr, cfg.input_file_name);
    if (err) return err;

    return SPU_ERROR_NO_ERROR;
}

SPUError SPU_dtor(SPU* spu_ptr)
{
    if (!spu_ptr) return SPU_ERROR_NULL_SPU_PTR;

    STACK_FUNC_WRAP( stack_dtor( &(spu_ptr->stk) ) );

    for (size_t ind = 0; ind < num_of_registers; ind++)
    {
        spu_ptr->registers[ind] = (int) 0xFFFFFFFF;
    }

    if (spu_ptr->cs) free(spu_ptr->cs);
    spu_ptr->cs_size = (size_t) -1;

    return SPU_ERROR_NO_ERROR;
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

    fprintf(stderr, "SPU REGISTERS:");
    for (size_t ind = 0; ind < num_of_registers; ind++)
    {
        fprintf(stderr, "\t[%s] : <%d>\n", registers_names[ind], spu_ptr->registers[ind]);
    }
}

inline void print_header_bytes_(const char* sign,
                                const int version,
                                const size_t binary_size_in_bytes)
{
    for (size_t ind = 0; ind < sizeof(int); ind++)
    {
        fprintf(stderr, "%02X ", sign[ind] );
    }

    for (size_t ind = 0; ind < sizeof(int); ind++)
    {
        fprintf(stderr, "%02X ", ( (char*) &version )[ind] );
    }

    for (size_t ind = 0; ind < sizeof(int); ind++)
    {
        fprintf(stderr, "%02X ", ( (char*) &binary_size_in_bytes )[ind] );
    }
}

inline void print_spu_header_and_cs_(SPU *spu_ptr)
{
    assert(spu_ptr);

    fprintf(stderr, "HEADER + CODE SEGMENT:\n{\n");

    print_header_bytes_(SIGN, VERSION, spu_ptr->cs_size + HEADER_SIZE_IN_BYTES);

    const size_t row_width = 16;

    size_t rows_count = (spu_ptr->cs_size + HEADER_SIZE_IN_BYTES) / row_width + ( spu_ptr->cs_size % row_width != 0 );
    for (size_t row = 0; row < rows_count; row++ )
    {
        for (size_t col = 0; col < row_width; col++)
        {
            if (row == 0 && col == 0)
            {
                // because header is already printed
                col = HEADER_SIZE_IN_BYTES;
            }

            // если вышли за границу массива то break!!!
            if ( row*row_width + col - HEADER_SIZE_IN_BYTES >= spu_ptr->cs_size )
                break;

            fprintf(stderr, "%02X ",
                    spu_ptr->cs[row*row_width + col - HEADER_SIZE_IN_BYTES]);
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

    if (spu_ptr->cs)
    {
        print_spu_header_and_cs_(spu_ptr);
    }

    fprintf(stderr, "STACK DUMP:\n");
    STACK_DUMP(&spu_ptr->stk, stack_verify(&spu_ptr->stk));
    fprintf(stderr, "STACK DUMP END!\n");

    fprintf(stderr, "====================== DUMP END ======================\n");
}

void print_spu_error(SPUError err)
{
    assert(err);

    fprintf(stderr, "SPU ERROR: <%s>\n", spu_error_messages[(int) err]);
}

// TODO CHANGE!
inline void print_inprog_error(unsigned long line, const char *str)
{
    fprintf(stderr, "ERROR on line %ld! The line is:\n<%s>\n",
                    line, str);
}

SPUError run_program(SPU *spu_ptr, int *prog_res)
{
    SPU_CHECK(spu_ptr);
    assert(prog_res);


}

inline int get_arg(const char *line)
{
    assert(line);

    int x = 0;
    sscanf(line, "%*s %s", &x);

    return x;
}

SPUExecCmdRes exec_command(Stack *stk_p, const char *line, Command cmd)
{
    assert(line);
    assert(cmd != CMD_UNKNOWN);

    /*
    switch (cmd)
    {
    case CMD_PUSH:
        stack_push(stk_p, get_arg(line));
        break;
    case CMD_ADD:
        int pop1 = 0;
        int pop2 = 0;

        stack_pop(stk_p, &pop1);
        stack_pop(stk_p, &pop2);

        stack_push(stk_p, pop1 + pop2);
        break;
    case CMD_SUB:

        break;
    case CMD_MUL:

        break;
    case CMD_DIV:

        break;
    case CMD_IN:

        break;
    case CMD_OUT:

        break;
    case CMD_HLT:
        return EXEC_CMD_RES_HLT;
        break;
    default:
        break;
    }
    */

    return EXEC_CMD_RES_DEFAULT;
}
