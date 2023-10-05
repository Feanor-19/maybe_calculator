#include "config_asm.h"

const char* FLAG_HELP               = "-h";
const char* FLAG_INPUT_FILE         = "-i";  // -i
const char* FLAG_OUTPUT_FILE        = "-o"; // -o

CmdLineFlag supported_flags[] = {
                                    { FLAG_HELP             , 0, 0, "" },
                                    { FLAG_INPUT_FILE       , 0, 1, "" },
                                    { FLAG_OUTPUT_FILE      , 0, 1, "" }
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

    if ( extract(N_FLAGS, supported_flags, FLAG_HELP)->state )
    {
        printf("%s\n", help_message);
        exit(0);
    }

    Config cfg = assemble_config(N_FLAGS, supported_flags);
    cfg.unread_flags = unread_flags;
    return cfg;
}

static Config assemble_config(size_t n_flags, CmdLineFlag flags[])
{
    assert(flags != NULL);

    Config config = {"", "", CONFIG_NO_ERROR};

    CmdLineFlag *p_curr_flag = NULL;

    if ( ( p_curr_flag = extract(n_flags, supported_flags, FLAG_INPUT_FILE) )!= NULL
       && p_curr_flag->state )
    {
        if ( is_str_empty(p_curr_flag->add_arg) )
        {
            config.error = CONFIG_ERROR_DATA_SOURCE;
            return config;
        }
        config.data_source = p_curr_flag->add_arg;
    }
    else
    {
        config.data_source = FILE_IN_DEFAULT_NAME;
    }

    if ( (p_curr_flag = extract(n_flags, supported_flags, FLAG_OUTPUT_FILE) )!= NULL
       && p_curr_flag->state )
    {
        if ( is_str_empty(p_curr_flag->add_arg) )
        {
            config.error = CONFIG_ERROR_OUTPUT_DESTINATION;
            return config;
        }
        config.output_destination = p_curr_flag->add_arg;
    }
    else
    {
        config.output_destination = FILE_OUT_DEFAULT_NAME;
    }

    return config;

}

void print_config(FILE *stream, Config cfg)
{
    assert(stream != NULL);
    assert(cfg.error == CONFIG_NO_ERROR);

    printf("The following configuration is set:\n"
    "data source:                               <%s>\n"
    "output destination:                        <%s>\n",
    cfg.data_source, cfg.output_destination);
}

void print_cfg_error_message(FILE *stream, ConfigError error)
{
    assert(stream != NULL);

    printf("Some error occured during confgiruation of the prorgam: ");

    switch (error)
    {
    case CONFIG_ERROR_DATA_SOURCE:
        fprintf(stream, "Data source (input file) error.\n");
        break;
    case CONFIG_ERROR_OUTPUT_DESTINATION:
        fprintf(stream, "Output destination (output file) error.\n");
        break;
    case CONFIG_NO_ERROR:
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
