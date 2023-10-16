/*
    Аргументы макроса DEF_CMD(name, id, is_im_const, is_reg, is_mem, ...):
        - имя команды
        - номер (id)
        - требует ли команда аргумент типа immediate const   [1/0]
        - требует ли команда аргумент типа register          [1/0]
        - требует ли команда аргумент типа memory            [1/0]
        - код команды для процессора (передается как __VA_LIST__)
*/

//---------------DSL------------------



//---------------CMD------------------

DEF_CMD(UNKNOWN,    0,  0,  0,  0, {
    return SPU_STATUS_ERROR_UNKNOWN_CMD;
})

DEF_CMD(PUSH,       1,  1,  1,  0, {

})

DEF_CMD(POP,        2,  0,  1,  0, {})

DEF_CMD(ADD,        3,  0,  0,  0, {})

DEF_CMD(SUB,        4,  0,  0,  0, {})

DEF_CMD(MUL,        5,  0,  0,  0, {})

DEF_CMD(DIV,        6,  0,  0,  0, {})

DEF_CMD(IN,         7,  0,  0,  0, {})

DEF_CMD(OUT,        8,  0,  0,  0, {})

DEF_CMD(HLT,        9,  0,  0,  0, {
    return SPU_STATUS_HLT;
})
