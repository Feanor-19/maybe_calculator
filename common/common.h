#ifndef COMMON_H
#define COMMON_H

/*
    ДАННАЯ ИНФОРМАЦИЯ ЯВЛЯЕТСЯ ПЕРВОИСТОЧНИКОМ. ВСЁ, ЧТО РАСПОЛОЖЕНО НИЖЕ, ДОЛЖНО ЕЙ СООТВЕТСТВОВАТЬ.
    ВЫ БЫЛИ ПРЕДУПРЕЖДЕНЫ.

    Структура бинарного файла:
        - заголовок длиной, которая вычисляется в константе HEADER_SIZE_IN_BYTES:
            - 4 байта сигнатуры (массив из 4 int8_t)
            - 1 байт под номер версии (uint8_t)
            - 4 байта под размер в байтах всего бинарного файла (uint32_t)
        - последовательность комманд, записанных в виде блоков байт переменной длины
            - вид одного блока байт:
            байт с номером команды, байт доп информации о команде, [аргумент]

            Пояснение:
            - номер команды соответсвует номеру из enum Command и записан как int8_t
            - структура байта информации о команде:
                номер бита:          назначение:

                    - b0             immediate const
                    - b1             register
                    - b2             memory
                    - b3             \
                    - b4             } номер регистра := b3 + b4*2 + b5*4
                    - b5             /
                    - b6             пока не используется
                    - b7             пока не используется

            - тип аргумента определяется на основании байта доп информации:
                включенный бит:         тип аргумента:

                - immediate const       int32_t
                - register              нет аргумента
                - memory                //TODO
*/

typedef int spu_stack_elem_t;

typedef int32_t immediate_const_t;

typedef int32_t BIN_HEADER_SIGN_t;
typedef uint8_t BIN_HEADER_VERSION_t;
typedef uint32_t BIN_HEADER_FILE_SIZE_t;

const uint8_t BIT_IMMEDIATE_CONST   = 0;
const uint8_t BIT_REGISTER          = 1;
const uint8_t BIT_MEMORY            = 2;

const uint8_t BIT_REG_ID_START      = 3; //< Register id starts on this bit
const uint8_t BIT_REG_ID_END        = 5; //< Register id ends on this bit

enum Command
{
    CMD_UNKNOWN = 0,
    CMD_PUSH    = 1,
    CMD_POP     = 2,
    CMD_ADD     = 3,
    CMD_SUB     = 4,
    CMD_MUL     = 5,
    CMD_DIV     = 6,
    CMD_IN      = 7,
    CMD_OUT     = 8,
    CMD_HLT     = 9,
};

//! @note MUST BE IN SYNC WITH ENUM COMMANDS!
const char *commands_list[] =
{
    "",     //CMD_UNKNOWN
    "push", //CMD_PUSH
    "pop",  //CMD_POP
    "add",  //CMD_ADD
    "sub",  //CMD_SUB
    "mul",  //CMD_MUL
    "div",  //CMD_DIV
    "in",   //CMD_IN
    "out",  //CMD_OUT
    "hlt"   //CMD_HLT
};

//! @note MUST BE IN SYNC WITH ENUM COMMANDS!
const int command_needs_im_const_arg[] =
{
    0,  //CMD_UNKNOWN
    1,  //CMD_PUSH
    0,  //CMD_POP
    0,  //CMD_ADD
    0,  //CMD_SUB
    0,  //CMD_MUL
    0,  //CMD_DIV
    0,  //CMD_IN
    0,  //CMD_OUT
    0   //CMD_HLT
};

//! @note MUST BE IN SYNC WITH ENUM COMMANDS!
const int command_needs_register_arg[] =
{
    0,  //CMD_UNKNOWN
    1,  //CMD_PUSH
    1,  //CMD_POP
    0,  //CMD_ADD
    0,  //CMD_SUB
    0,  //CMD_MUL
    0,  //CMD_DIV
    0,  //CMD_IN
    0,  //CMD_OUT
    0   //CMD_HLT
};

const size_t commands_list_len = sizeof(commands_list)/sizeof(commands_list[0]);

//--- BINARY FILE HEADER
const static char SIGN_RAW_[4] = {'S', 'F', '1', '9'};
const BIN_HEADER_SIGN_t SIGN = *((const BIN_HEADER_SIGN_t*) SIGN_RAW_);
const BIN_HEADER_VERSION_t VERSION   = 6;
const size_t HEADER_SIZE_IN_BYTES = sizeof(SIGN) + sizeof(VERSION) + sizeof(BIN_HEADER_FILE_SIZE_t);
//---

const spu_stack_elem_t COMPUTATIONAL_MULTIPLIER = 1000;

const size_t register_name_len = 3;
const char registers_names[][register_name_len + 1] =
{
    "rax",
    "rbx",
    "rcx",
    "rdx"
};

const size_t num_of_registers = sizeof(registers_names)/sizeof(registers_names[0]);
//-------------------------------------------------------------------------------------------------------

#define sizearr(arr) sizeof(arr)/sizeof(arr[0])

static_assert(sizearr(command_needs_im_const_arg) == sizearr(command_needs_register_arg),
                "command_needs_im_const_arg's and command_needs_register_arg's sizes are snot equal!");

static_assert(sizeof(SIGN) == sizeof(int), "SIGN must be as long as int is!");

#undef sizearr

#endif /* COMMON_H */
