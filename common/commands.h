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

#define SPU spu_ptr
#define IP spu_ptr->ip
#define CS spu_ptr->cs
#define REGISTERS spu_ptr->registers
#define PROG_RES_PTR prog_res

#define PUSH_(x) stack_push( &SPU->stk, (x) )
#define POP_(POINTER) stack_pop( &SPU->stk, (POINTER) )

// можно ли вместо этого дефайна использовать inline функцию?
// тогда она окажется объявлена посреди enum, внутри массивов...
// можно сделать inline в spu
#define GET_REG_(INFO_BYTE_, REG_) do                                        \
{                                                                           \
    REG_ = 0;                                                               \
    for ( uint8_t bit = BIT_REG_ID_START; bit <= BIT_REG_ID_END; bit++ )    \
    {                                                                       \
        if ( (uint8_t) (INFO_BYTE_) & bit )                          \
            (REG_) = set_bit( (REG_), bit - BIT_REG_ID_START);      \
    }                                                                       \
} while (0)                                                                 \

//---------------CMD------------------

DEF_CMD(UNKNOWN,    0,  0,  0,  0, {
    return SPU_STATUS_ERROR_UNKNOWN_CMD;
})

DEF_CMD(PUSH,       1,  1,  1,  0, {
    SPU_CHECK(SPU);

    IP++;

    if ( CS[IP] & BIT_IMMEDIATE_CONST )
    {
        IP++;
        spu_stack_elem_t im_const = *((spu_stack_elem_t *) (CS + IP));
        STACK_FUNC_WRAP( PUSH_(im_const) );
        IP += sizeof(SPU_STACK_ELEM_T);
    }
    else if ( CS[IP] & BIT_REGISTER )
    {
        uint8_t REG = 0;
        GET_REG_( (CS[IP]), REG );
        STACK_FUNC_WRAP( PUSH_( REGISTERS[REG] ) );
        IP++;
    }
    else
    {
        return SPU_STATUS_ERROR_UNKNOWN_CMD;
    }

    return SPU_STATUS_OK;
})

DEF_CMD(POP,        2,  0,  1,  0, {
    SPU_CHECK(SPU);

    IP++;

    if ( CS[IP] & BIT_REGISTER )
    {
        uint8_t REG = 0;
        GET_REG_( (CS[IP]), REG );
        STACK_FUNC_WRAP( POP_( &(REGISTERS[REG]) ) );
        IP++;
    }
    else
    {
        return SPU_STATUS_ERROR_UNKNOWN_CMD;
    }

    return SPU_STATUS_OK;
})

DEF_CMD(ADD,        3,  0,  0,  0, {
    SPU_CHECK(SPU);

    spu_stack_elem_t a = 0, b = 0;
    STACK_FUNC_WRAP( POP_( &(a) ) );
    STACK_FUNC_WRAP( POP_( &(b) ) );
    STACK_FUNC_WRAP( PUSH_( a + b ) );

    IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(SUB,        4,  0,  0,  0, {
    SPU_CHECK(SPU);

    spu_stack_elem_t a = 0, b = 0;
    STACK_FUNC_WRAP( POP_( &(a) ) );
    STACK_FUNC_WRAP( POP_( &(b) ) );
    STACK_FUNC_WRAP( PUSH_( b - a ) );

    IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(MUL,        5,  0,  0,  0, {
    SPU_CHECK(SPU);

    spu_stack_elem_t a = 0, b = 0;
    STACK_FUNC_WRAP( POP_( &(a) ) );
    STACK_FUNC_WRAP( POP_( &(b) ) );
    STACK_FUNC_WRAP( PUSH_( (a * b) / COMPUTATIONAL_MULTIPLIER ) );

    IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(DIV,        6,  0,  0,  0, {
    SPU_CHECK(SPU);

    spu_stack_elem_t a = 0, b = 0;
    STACK_FUNC_WRAP( POP_( &(a) ) );
    STACK_FUNC_WRAP( POP_( &(b) ) );
    if (b == 0)
        return SPU_STATUS_ERROR_DIV_BY_ZERO;
    STACK_FUNC_WRAP( PUSH_( (a / b) * COMPUTATIONAL_MULTIPLIER ) );

    IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(IN,         7,  0,  0,  0, {
    SPU_CHECK(SPU);

    double in = 0;

    fprintf(stdout, "Please enter 'in':\n");
    if ( fscanf(stdin, "%lf", &in) != 1 )
        return SPU_STATUS_ERROR_WRONG_IN;

    STACK_FUNC_WRAP( PUSH_( (spu_stack_elem_t) (in * COMPUTATIONAL_MULTIPLIER) ) );

    IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(OUT,        8,  0,  0,  0, {
    SPU_CHECK(SPU);
    assert( PROG_RES_PTR );

    spu_stack_elem_t buf = 0;
    STACK_FUNC_WRAP( POP_( &buf ) );
    *PROG_RES_PTR = ( (double) buf ) / COMPUTATIONAL_MULTIPLIER;

    IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(HLT,        9,  0,  0,  0, {
    return SPU_STATUS_HLT;
})

//-------------UNDEF DSL--------------

#undef SPU
#undef IP
#undef CS
#undef REGISTERS
#undef PROG_RES_PTR

#undef PUSH_
#undef POP_

#undef GET_REG_
