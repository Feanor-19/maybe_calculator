#ifndef CONFIG_H
#define CONFIG_H

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/parser_cmd_args.h"

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    HOW TO ADD A NEW FLAG
    1) Add a new member in struct Config in config.h (if needed)
    2) Add a new element in supported_flags[] in config.cpp
    3) Add a new constant string right before supported_flags[] in config.cpp
    4) Modify get_config(), assemble_config(), print_config() in config.cpp
    5) If needed, add a new member in ConfigError enum in config.h and modify print_cfg_error_message()
    6) Profit!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

//-------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------

enum ConfigError
{
    CONFIG_NO_ERROR     = 0,
    CONFIG_ERROR_INPUT  = 1,
    CONFIG_ERROR_OUTPUT = 2,
    CONFIG_ERROR_PARAMS = 3,
};

/*!
    @brief Contains all configurable parametres of the automaton.
*/
struct Config
{
    const char *input_file_name;        //!< Name of the file with program.
    const char *output_file_name;       //!< Name of the file where to put translated to binary program.
    const char *params_file_name;       //!< Name of the file to get params for command 'in' from.
    ConfigError error;                  //!< ConfigError enum value.
    int unread_flags;                   //!< Number of unrecognized flags.
};

//! @brief Gets all configurable parametres of the automaton.
//! @param argc
//! @return All collected information packed in the Config struct.
Config get_config(int argc, const char *argv[]);

void print_config(FILE *stream, Config cfg);

void print_cfg_error_message(FILE *stream, ConfigError error);

int is_str_empty(const char *str);

#endif
