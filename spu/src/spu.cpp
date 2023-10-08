#include <stdio.h>

#include "config_spu.h"

typedef int Elem_t;
inline void print_elem_t(FILE *stream, Elem_t val) {fprintf(stream, "%d", val);}
#define STACK_DO_DUMP
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

    Input program = read_input_file(cfg.input_file_name);
    if (program.err) return program.err;



    free_struct_input(program);

    return 0;
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
        input.err = SPU_ERROR_READ_INPUT_FILE;
        return input;
    }

    Text text = parse_buf_to_text(file_buf);

    input.text = text;
    input.file_buf = file_buf;
    return input;
}

inline void print_spu_error(unsigned long line, const char *str)
{
    fprintf(stderr, "ERROR on line %ld! The line is:\n<%s>\n",
                    line, str);
}

SPUError run_program(Input prog, int *prog_res)
{
    ASSERT_INPUT(prog);
    assert(prog_res);

    Stack stk = {};
    stack_ctor(&stk);

    for (unsigned long ind = 0; ind < prog.text.nLines; ind++)
    {
        Command cmd = CMD_UNKNOWN;
        if ( sscanf(prog.text.line_array[ind], "%d", &cmd) != 1 )
        {
            print_spu_error(ind + 1, prog.text.line_array[ind]);
            return SPU_ERROR_IN_PROG;
        }

        SPUExecCmdRes res = exec_command(&stk, prog, cmd);
        if (res == EXEC_CMD_RES_HLT)
        {
            stack_pop(&stk, prog_res);
            return SPU_ERROR_NO_ERROR;
        }
    }

    stack_dtor(&stk);
}

inline int get_arg(const char *line)
{
    assert(line);

    int x = 0;
    sscanf(line, "%*s %s", &x);

    return x;
}

inline void cmd_push(Stack *stk_p, const char* line)
{

}

inline void cmd_add(Stack *stk_p, const char* line)
{

}

inline void cmd_sub(Stack *stk_p, const char* line)
{

}

inline void cmd_push(Stack *stk_p, const char* line)
{

}

inline void cmd_push(Stack *stk_p, const char* line)
{

}

inline void cmd_push(Stack *stk_p, const char* line)
{

}

inline void cmd_push(Stack *stk_p, const char* line)
{

}


SPUExecCmdRes exec_command(Stack *stk_p, const char *line, Command cmd)
{
    assert(line);
    assert(cmd != CMD_UNKNOWN);

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

    return EXEC_CMD_RES_DEFAULT;
}

void free_struct_input(Input input)
{
    ASSERT_INPUT(input);

    free(input.file_buf.buf);
    input.file_buf.buf = NULL;
    input.file_buf.buf_size = 0;

    free(input.text.line_array);
    input.text.line_array = NULL;
    input.text.nLines = 0;
}
