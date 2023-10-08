#include "parser_cmd_args.h"

int parse_cmd_args(int argc, const char * argv[], size_t n_flags, CmdLineFlag flags[])
{
    assert(argv != NULL);
    assert(flags != NULL);
    assert(n_flags > 0);

    int skipped_args = 0;

    //пропускаем первый аргумент
    argc-=1;
    argv++;

    while( argc-- )
    {
        int flag_found = 0;

        for (size_t ind = 0; ind < n_flags; ind++)
        {
            if ( flags[ind].flag == NULL ) continue;

            if ( strcmp(*argv, flags[ind].flag) == 0 )
            {

                flag_found = 1;

                flags[ind].state = 1;

                if ( flags[ind].needs_add_arg )
                {
                    if (argc != 0)
                    {
                        ++argv;
                        argc--;

                        assert((flags[ind]).add_arg != NULL);

                        strcpy((flags[ind]).add_arg, *argv);
                    }
                    else
                    {
                        // доп аргумент не задан
                        ((flags[ind]).add_arg)[0] = '\0';
                    }
                }
                else
                {
                    ((flags[ind]).add_arg)[0] = '\0';
                }

                break; //не может аргумент равняться двум флагам сразу
            }
        }

        if (!flag_found) skipped_args++;

        ++argv;
    }

    return skipped_args;
}

CmdLineFlag* extract(size_t n_flags, CmdLineFlag flags[], const char* flag)
{
    assert(flags != NULL);
    assert(n_flags > 0);
    assert(flag != NULL);
    assert(flag[0] != '\0');

    for (size_t ind = 0; ind < n_flags; ind++)
    {
        if ( strcmp(flags[ind].flag, flag) == 0 )
        {
            return flags + ind;
        }
    }

    return NULL;
}
