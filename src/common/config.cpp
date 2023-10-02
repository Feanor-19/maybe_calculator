#include "config.h"

const char* FLAG_HELP               = "-h";
const char* FLAG_TESTS              = "-tests"; // --run-test
const char* FLAG_INPUT_FILE         = "-fin";  // -i
const char* FLAG_OUTPUT_FILE        = "-fout"; // -o
const char* FLAG_PRINT_ADDRESS      = "-addr"; // --print-address
const char* FLAG_SORT_FROM_BEGIN    = "-sb";
const char* FLAG_SORT_FROM_END      = "-se";

CmdLineFlag supported_flags[] = {
                                    { FLAG_HELP             , 0, 0, "" },
                                    { FLAG_TESTS            , 0, 0, "" },
                                    { FLAG_INPUT_FILE       , 0, 1, "" },
                                    { FLAG_OUTPUT_FILE      , 0, 1, "" },
                                    { FLAG_PRINT_ADDRESS    , 0, 0, "" },
                                    { FLAG_SORT_FROM_BEGIN  , 0, 0, "" },
                                    { FLAG_SORT_FROM_END    , 0, 0, "" }
                                };

const char *help_message = "No help message yet :-(";

/*!
    @brief Assembles struct Config from array of structs CmdLineFlag.
    @param [in] n_flags Length of array flags[].
    @param [in] flags[] Array of structs CmdLineFlag.
    @return Assembled struct Config. If some error occurred,
    Config.error is set to ConfigError enum value.
*/
static Config assemble_config(size_t n_flags, CmdLineFlag flags[]);

//----------------------------------------------------------------

Config get_config(int argc, const char *argv[])
{
    assert(argc > 0);
    assert(argv != NULL);

    const size_t N_FLAGS = sizeof(supported_flags)/sizeof(supported_flags[0]);

    int unread_flags = parse_cmd_args(argc, argv, N_FLAGS, supported_flags);

    if ( unread_flags > 0 ) printf("WARNING: %d flags were unrecognized.\n", unread_flags);

    if ( extract(N_FLAGS, supported_flags, FLAG_HELP)->state )
    {
        printf("%s\n", help_message);
        exit(0);
    }

    return assemble_config(N_FLAGS, supported_flags);
}

static Config assemble_config(size_t n_flags, CmdLineFlag flags[])
{
    assert(flags != NULL);

    Config config = {0, "", "", 0, 0, NO_ERROR};

    CmdLineFlag *p_curr_flag = NULL;

    if ( ( p_curr_flag = extract(n_flags, supported_flags, FLAG_TESTS) )!= NULL
       && p_curr_flag->state )
    {
        config.do_tests = 1;
    }

    if ( ( p_curr_flag = extract(n_flags, supported_flags, FLAG_INPUT_FILE) )!= NULL
       && p_curr_flag->state )
    {
        if ( is_str_empty(p_curr_flag->add_arg) )
        {
            config.error = ERROR_DATA_SOURCE;
            return config;
        }
        config.data_source = p_curr_flag->add_arg;
    }

    if ( (p_curr_flag = extract(n_flags, supported_flags, FLAG_OUTPUT_FILE) )!= NULL
       && p_curr_flag->state )
    {
        if ( is_str_empty(p_curr_flag->add_arg) )
        {
            config.error = ERROR_OUTPUT_DESTINATION;
            return config;
        }
        config.output_destination = p_curr_flag->add_arg;
    }

    if ( (p_curr_flag = extract(n_flags, supported_flags, FLAG_PRINT_ADDRESS) )!= NULL
       && p_curr_flag->state )
    {
        config.do_print_addresses = 1;
    }

    config.do_sort_begin = 0;
    config.do_sort_end = 0;

    if ( (p_curr_flag = extract(n_flags, supported_flags, FLAG_SORT_FROM_BEGIN) )!= NULL
       && p_curr_flag->state )
    {
        config.do_sort_begin = 1;
    }

    if ( (p_curr_flag = extract(n_flags, supported_flags, FLAG_SORT_FROM_END) )!= NULL
       && p_curr_flag->state )
    {
        config.do_sort_begin = 0;
        config.do_sort_end = 1;
    }

    return config;

}

void print_config(Config cfg, FILE *stream)
{
    assert(stream != NULL);
    assert(cfg.error == NO_ERROR);

    printf("The following configuration is set:\n"
    "data source:                               <%s>\n"
    "output destination:                        <%s>\n"
    "do sorting from the beginning of a line:   <%s>\n"
    "do sorting from the end of a line:         <%s>\n",
    cfg.data_source, cfg.output_destination,
    ( cfg.do_sort_begin ? "yes" : "no" ), ( cfg.do_sort_end ? "yes" : "no" ) );
}

void print_cfg_error_message(FILE *stream, ConfigError error)
{
    assert(stream != NULL);

    printf("Some error occured during confgiruation of the automoton: ");

    switch (error)
    {
    case ERROR_DATA_SOURCE:
        printf("Data source (input file) error.\n");
        break;
    case ERROR_OUTPUT_DESTINATION:
        printf("Output destination (output file) error.\n");
        break;
    case NO_ERROR:
    default:
        assert(0 && "Default case in ConfigError switch!");
        break;
    }

    printf("Please, restart and try again.\n");

    return;
}

int is_str_empty(const char *str)
{
    assert( str != NULL );

    return str[0] == '\0';
}
