#ifndef SPU_H
#define SPU_H

#include "../../common/onegin.h"
#include "../../common/commands.h"

/*
    USED DEFINES:
#define SPU_DO_DUMP

*/

const char* spu_error_messages[] =
{
    "SPU_ERROR_NO_ERROR",
    "SPU_ERROR_GET_IN_OUT_FILES_NAMES",
    "SPU_ERROR_READ_INPUT_FILE",
    "SPU_ERROR_IN_PROG",
    "SPU_ERROR_NULL_SPU_PTR",
    "SPU_ERROR_CONFIG_HAS_ERROR",
    "SPU_ERROR_MEM_ALLOC_ERROR",
    "SPU_ERROR_INP_FILE_HEADER_CORRUPTED",
    "SPU_ERROR_ZERO_BYTES_OF_CODE",
    "SPU_ERROR_HEADER_WRONG_SIGN",
    "SPU_ERROR_INPUT_FILE_WRONG_VERSION",
    "SPU_ERROR_INPUT_FILE_CODE_CORRUPTED",
    "SPU_ERROR_STACK_ERROR"
};

enum SPUError
{
    SPU_ERROR_NO_ERROR                  = 0,
    SPU_ERROR_GET_IN_OUT_FILES_NAMES    = 1,
    SPU_ERROR_READ_INPUT_FILE           = 2,
    SPU_ERROR_IN_PROG                   = 3,
    SPU_ERROR_NULL_SPU_PTR              = 4,
    SPU_ERROR_CONFIG_HAS_ERROR          = 5,
    SPU_ERROR_MEM_ALLOC_ERROR           = 6,
    SPU_ERROR_INP_FILE_HEADER_CORRUPTED = 7,
    SPU_ERROR_ZERO_BYTES_OF_CODE        = 8,
    SPU_ERROR_HEADER_WRONG_SIGN         = 9,
    SPU_ERROR_INPUT_FILE_WRONG_VERSION  = 10,
    SPU_ERROR_INPUT_FILE_CODE_CORRUPTED = 11,
    SPU_ERROR_STACK_ERROR               = 12,
};

enum SPUExecCmdRes
{
    EXEC_CMD_RES_DEFAULT    = 0,
    EXEC_CMD_RES_HLT        = 1,
};

struct SPU
{
    Stack stk = {};

    int registers[num_of_registers] = {};

    char *cs = NULL;
    size_t cs_size = 0;
    size_t ip = 0;
};

//-------------------------------------------------------------------------------------------------------------

const char *spu_verification_messages[] =
{
    "SPU_VERIFY_NULL_PNT",
    "SPU_VERIFY_STACK_VERIFY_ERROR",
    "SPU_VERIFY_CS_PTR_NULL",
    "SPU_VERIFY_CS_SIZE_TOO_BIG",
    "SPU_VERIFY_IP_VALUE_INVALID"
};

//! @note These flags must not collide with StackVerifyResFlag!
enum SPUVerifyRes
{
    SPU_VERIFY_NULL_PNT             = 1 << 16,
    SPU_VERIFY_STACK_VERIFY_ERROR   = 1 << 17,
    SPU_VERIFY_CS_PTR_NULL          = 1 << 18,
    SPU_VERIFY_CS_SIZE_TOO_BIG      = 1 << 19,
    SPU_VERIFY_IP_VALUE_INVALID     = 1 << 20,
};

const size_t SPU_FIRST_VERIFY_RES_ID = 16;
const size_t NUMBER_OF_SPU_VERIFY_RES_FLAGS = 5;
static_assert(sizeof(spu_verification_messages)/sizeof(spu_verification_messages[0]) == NUMBER_OF_SPU_VERIFY_RES_FLAGS,
                "Number of verification messages isn't the same as number of verify flags!");

//-------------------------------------------------------------------------------------------------------------

SPUError SPU_ctor(SPU* spu_ptr, Config cfg);

SPUError SPU_dtor(SPU* spu_ptr);

int SPU_verificator(SPU* spu_ptr);

void SPU_dump_( SPU* spu_ptr,
                    const char* file_name,
                    int line,
                    const char* func_name,
                    int verify_res = 0);

#define SPU_DUMP(spu_ptr, verify_res) SPU_dump_(spu_ptr, __FILE__, __LINE__, __func__, verify_res)

#define SPU_CHECK(spu_ptr) do                       \
{                                                   \
    int verify_res = SPU_verificator(spu_ptr);      \
    if ( verify_res != 0 )                          \
    {                                               \
        SPU_DUMP(spu_ptr, verify_res);              \
    }                                               \
} while (0);                                        \

void print_spu_error(SPUError err);

#define PRINT_IF_SPU_ERROR_(err) do { if ((err)) { print_spu_error((err)); return (err); } } while(0)

void print_spu_verify_res_(int verify_res);

SPUError run_program(SPU *spu_ptr, int *prog_res);

SPUExecCmdRes exec_command(Stack *stk_p, const char *line, Command cmd);

//! @note BE VERY CAREFUL USING THIS! MAYBE NOT ALL STACK
//! FUNCTIONS RETURN STACK_ERROR, ALSO BE AWARE THAT
//! STACK_ERROR ITSELF IS LOST!
#define STACK_FUNC_WRAP(stack_func)                 \
do{                                                 \
    StackErrorCode stk_err = (stack_func);          \
    if ( (stk_err) ) return SPU_ERROR_STACK_ERROR;  \
}while(0)                                           \

#endif /* SPU_H */
