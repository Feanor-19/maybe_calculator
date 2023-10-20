/*
    Аргументы макроса DEF_CMD(name, id, is_im_const, is_reg, is_mem, is_label, ...):
        - имя команды
        - номер (id)
        - требует ли команда аргумент типа immediate const   [1/0]
        - требует ли команда аргумент типа register          [1/0]
        - требует ли команда аргумент типа memory            [1/0]
        - код команды для процессора (передается как __VA_LIST__)
*/

/*
    ПОЛЕЗНОЕ ДЛЯ ДЕБАГА:

    g++ -E -o spu_processed.cpp spu.cpp
    grep.exe -v '^#' .\spu_processed.cpp > spu_ready.cpp
*/

//---------------DSL------------------

#define _SPU spu_ptr
#define _IP spu_ptr->ip
#define _CS spu_ptr->cs
#define _REGISTERS spu_ptr->registers
#define _PROG_RES_PTR prog_res

#define _IM_CONST_T     spu_stack_elem_t
#define _REG_T          uint8_t
#define _PROG_RES_T     double
#define _CS_OFFSET_T    cs_offset_t

#define _PROG_RES_T_SPECF "%lf"

#define _ADD(a, b) ((a) + (b))
#define _SUB(a, b) ((a) - (b))
#define _DIV(a, b) ( ((a) / (b)) * COMPUTATIONAL_MULTIPLIER)
#define _MUL(a, b) ( ((a) * (b)) / COMPUTATIONAL_MULTIPLIER)

#define _CAST_PROG_RES_TO_IM_CONST( x_ ) ( (_IM_CONST_T) (x_ * COMPUTATIONAL_MULTIPLIER) )
#define _CAST_IM_CONST_TO_PROG_RES( x_ ) ( ( (_PROG_RES_T) buf ) / COMPUTATIONAL_MULTIPLIER )

#define _PUSH(x) STACK_FUNC_WRAP(stack_push( &(_SPU->stk), (x) ))
#define _POP(ptr_to) STACK_FUNC_WRAP(stack_pop( &(_SPU->stk), (ptr_to) ))

#define _CHECK_SPU SPU_CHECK(_SPU)
#define _GET_INFO_BYTE _CS[_IP+1]
#define _IS_ARG_IM_CONST() test_bit(_GET_INFO_BYTE, BIT_IMMEDIATE_CONST)
#define _IS_ARG_REG() test_bit(_GET_INFO_BYTE, BIT_REGISTER)
#define _IS_ARG_MEM() test_bit(_GET_INFO_BYTE, BIT_MEMORY)
#define _GET_ARG_IM_CONST()  *((_IM_CONST_T *) (_CS + _IP + 2))
#define _GET_ARG_REG(REG_PTR_) do                                               \
{                                                                           \
    *REG_PTR_ = 0;                                                          \
    for ( _REG_T bit = BIT_REG_ID_START; bit <= BIT_REG_ID_END; bit++ )     \
    {                                                                       \
        if ( (_REG_T) (_GET_INFO_BYTE) & (1 << bit) )                       \
            (*REG_PTR_) = set_bit( (*REG_PTR_), bit - BIT_REG_ID_START);    \
    }                                                                       \
} while (0)
#define _GET_CS_OFFSET() *((_CS_OFFSET_T *) (_CS + _IP + 2))
#define _CLEAR_IN_BUF() while( getc(stdin) != '\n' )                                                             \

//---------------CMD------------------

DEF_CMD(UNKNOWN,    0,  0,  0,  0,  0, {
    return SPU_STATUS_ERROR_UNKNOWN_CMD;
})

DEF_CMD(PUSH,       1,  1,  1,  0,  0, {
    if ( _IS_ARG_IM_CONST() )
    {
        _PUSH(_GET_ARG_IM_CONST());
        _IP += 2 + sizeof(_IM_CONST_T);
    }
    else if ( _IS_ARG_REG() )
    {
        _REG_T reg = 0;
        _GET_ARG_REG( &reg );
        _PUSH( _REGISTERS[reg] );
        _IP += 2;
    }
    else
    {
        return SPU_STATUS_ERROR_UNKNOWN_CMD;
    }

    return SPU_STATUS_OK;
})

DEF_CMD(POP,        2,  0,  1,  0,  0,{
    if ( _IS_ARG_REG() )
    {
        _REG_T reg = 0;
        _GET_ARG_REG( &reg );
        _POP(&(_REGISTERS[reg]));
        _IP += 2;
    }
    else
    {
        return SPU_STATUS_ERROR_UNKNOWN_CMD;
    }

    return SPU_STATUS_OK;
})

DEF_CMD(ADD,        3,  0,  0,  0,  0,{
    _IM_CONST_T a = 0;
    _IM_CONST_T b = 0;
    _POP(&a);
    _POP(&b);
    _PUSH( _ADD(a, b) );

    _IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(SUB,        4,  0,  0,  0,  0,{
    _IM_CONST_T a = 0;
    _IM_CONST_T b = 0;
    _POP(&a);
    _POP(&b);
    _PUSH( _SUB(b, a) );

    _IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(MUL,        5,  0,  0,  0,  0,{
    _IM_CONST_T a = 0;
    _IM_CONST_T b = 0;
    _POP(&a);
    _POP(&b);
    _PUSH( _MUL(a,b) );

    _IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(DIV,        6,  0,  0,  0,  0,{
    _IM_CONST_T a = 0;
    _IM_CONST_T b = 0;
    _POP(&a);
    _POP(&b);
    if (b == 0)
        return SPU_STATUS_ERROR_DIV_BY_ZERO;
    _PUSH( _DIV(a,b) );

    _IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(IN,         7,  0,  0,  0,  0,{
    _PROG_RES_T in = 0;

    fprintf(stdout, "Please enter 'in':\n");
    if ( fscanf(stdin, _PROG_RES_T_SPECF, &in) != 1 )
        return SPU_STATUS_ERROR_WRONG_IN;

    _CLEAR_IN_BUF();

    _PUSH( _CAST_PROG_RES_TO_IM_CONST( in ) );

    _IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(OUT,        8,  0,  0,  0,  0,{
    _IM_CONST_T buf = 0;
    _POP(&buf);
    *_PROG_RES_PTR = _CAST_IM_CONST_TO_PROG_RES( buf );

    _IP++;

    return SPU_STATUS_OK;
})

DEF_CMD(HLT,        9,  0,  0,  0,  0,{
    return SPU_STATUS_HLT;
})

DEF_CMD(JMP,        10, 0,  0,  0,  1, {
    _IP = _GET_CS_OFFSET()

    return SPU_STATUS_OK;
})

//-------------UNDEF DSL--------------

#undef _SPU
#undef _IP
#undef _CS
#undef _REGISTERS
#undef _PROG_RES_PTR

#undef _IM_CONST_T
#undef _REG_T
#undef _PROG_RES_T

#undef _PROG_RES_T_SPECF

#undef _ADD
#undef _SUB
#undef _DIV
#undef _MUL

#undef _CAST_PROG_RES_TO_IM_CONST
#undef _CAST_IM_CONST_TO_PROG_RES

#undef _PUSH
#undef _POP

#undef _CHECK_SPU
#undef _GET_INFO_BYTE
#undef _IS_ARG_IM_CONST()
#undef _IS_ARG_REG()
#undef _IS_ARG_MEM()
#undef _GET_ARG_IM_CONST()
#undef _GET_ARG_REG
