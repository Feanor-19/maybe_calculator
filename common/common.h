#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

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
                номер бита:         назначение:

                    - b0            immediate const
                    - b1            register
                    - b2            memory !Примечание: не имеет смысла само по себе! нужны b0/b1
                    - b3            cs offset (смещение от начала сегмента с кодом)
                    - b4            \
                    - b5            } номер регистра := b3 + b4*2 + b5*4
                    - b6            /
                    - b7            пока не используется

                Примечание: если у команды нет аргумента, то байт информации всё равно
                есть, но он нулевой.

            - тип аргумента определяется на основании байта доп информации:
                включенный бит:         тип аргумента:

                - immediate const       int32_t
                - register              нет аргумента
                - memory                зависит от того, какие биты ещё включены (im_const/register)
                - cs offset             uint32_t

*/

typedef int spu_stack_elem_t;
// такой тип имеет результат программы и числовые константы (не адреса) в коде программ
typedef double extern_num_t;

typedef int32_t immediate_const_t; //TODO - исправить путаницу между im_const и spu_stack_elem_t, maybe inner_num_t
typedef uint32_t cs_offset_t; //!< jmp cmd argument
// typedef uint32_t memory_t; // кажется этого не нужно

typedef int32_t BIN_HEADER_SIGN_t;
typedef uint8_t BIN_HEADER_VERSION_t;
typedef uint32_t BIN_HEADER_FILE_SIZE_t;

const uint8_t BIT_IMMEDIATE_CONST   = 0;
const uint8_t BIT_REGISTER          = 1;
const uint8_t BIT_MEMORY            = 2;
const uint8_t BIT_CS_OFFSET         = 3;

const uint8_t BIT_REG_ID_START      = 4; //< Register id starts on this bit
const uint8_t BIT_REG_ID_END        = 6; //< Register id ends on this bit

//----------------------------------------------------------------------------------------------

#define DEF_CMD(name, id, is_im_const, is_reg, is_mem, is_label, ...) CMD_##name = id,
enum Command
{
    #include "commands.h"
};
#undef DEF_CMD

#define DEF_CMD(name, id, is_im_const, is_reg, is_mem, is_label, ...) #name,
const char *commands_list[] =
{
    #include "commands.h"
    "FICTIONAL_CMD!"
};
#undef DEF_CMD

#define DEF_CMD(name, id, is_im_const, is_reg, is_mem, is_label, ...) is_im_const,
const int command_needs_im_const_arg[] =
{
    #include "commands.h"
    0 // FICTIONAL_CMD!
};
#undef DEF_CMD

#define DEF_CMD(name, id, is_im_const, is_reg, is_mem, is_label, ...) is_reg,
const int command_needs_register_arg[] =
{
    #include "commands.h"
    0 // FICTIONAL_CMD!
};
#undef DEF_CMD

#define DEF_CMD(name, id, is_im_const, is_reg, is_mem, is_label, ...) is_mem,
const int command_needs_memory_arg[] =
{
    #include "commands.h"
    0 // FICTIONAL_CMD!
};
#undef DEF_CMD

#define DEF_CMD(name, id, is_im_const, is_reg, is_mem, is_label, ...) is_label,
const int command_needs_label_arg[] =
{
    #include "commands.h"
    0 // FICTIONAL_CMD!
};
#undef DEF_CMD

const size_t commands_list_len = sizeof(commands_list)/sizeof(commands_list[0]);

//--- BINARY FILE HEADER -----------------------------------------------------------------------------
const static char SIGN_RAW_[4] = {'S', 'F', '1', '9'};
const BIN_HEADER_SIGN_t SIGN = *((const BIN_HEADER_SIGN_t*) SIGN_RAW_);
const BIN_HEADER_VERSION_t VERSION   = 13;
const BIN_HEADER_FILE_SIZE_t HEADER_SIZE_IN_BYTES = sizeof(BIN_HEADER_SIGN_t) + sizeof(BIN_HEADER_VERSION_t) + sizeof(BIN_HEADER_FILE_SIZE_t);
//----------------------------------------------------------------------------------------------------

const spu_stack_elem_t COMPUTATIONAL_MULTIPLIER = 1000;
const cs_offset_t CS_OFFSET_POISON_VALUE = -1;
const size_t MEMORY_SIZE = 128;

//----------------------------------------------------------------------------------------------------

//! @note REGISTER'S NAMES MUST BE ONLY UPPERCASE!
const size_t register_name_len = 3;
const char registers_names[][register_name_len + 1] =
{
    "RAX",
    "RBX",
    "RCX",
    "RDX"
};

const size_t num_of_registers = sizeof(registers_names)/sizeof(registers_names[0]);
//-------------------------------------------------------------------------------------------------------

static_assert(sizeof(SIGN_RAW_) == sizeof(BIN_HEADER_SIGN_t), "SIGN must be as long as int is!");

#endif /* COMMON_H */
