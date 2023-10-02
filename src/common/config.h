#ifndef CONFIG_H
#define CONFIG_H

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser_cmd_args.h"

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    HOW TO ADD A NEW FLAG
    1) Add a new member in struct Config in config.h (if needed)
    2) Add a new element in supported_flags[] in config.cpp
    3) Add a new constant string right before supported_flags[] in config.cpp
    4) Modify get_config(), assemble_config(), print_config() in config.cpp
    5) If needed, add a new member in ConfigError enum in config.h and modify print_cfg_error_message()
    6) Profit!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

enum ConfigError
{
    NO_ERROR                    = 0,
    ERROR_DATA_SOURCE           = 1,
    ERROR_OUTPUT_DESTINATION    = 2,
};

/*!
    @brief Contains all configurable parametres of the automaton.
    @note do_gen (-g) rewrites do_parse (-p)!
*/
struct Config
{
    int do_tests;                       //!< Do tests and shut down (1 or 0).
    const char *data_source;            //!< Source of the data.
    const char *output_destination;     //!< Destination for the output.
    int do_print_addresses;             //!< Used in print_text_to_stream
    int do_sort_begin;                  //!< Do sort from the beginning of a line (1 or 0).
    int do_sort_end;                    //!< Do sort from the end of a line (1 or 0). Rewrites do_sort_begin!
    ConfigError error;                  //!< ConfigError enum value.
};

//! @brief Gets all configurable parametres of the automaton.
//! @param argc
//! @return All collected information packed in the Config struct.
Config get_config(int argc, const char *argv[]);

void print_config(Config cfg, FILE *stream);

void print_cfg_error_message(FILE *stream, ConfigError error);

int is_str_empty(const char *str);

#endif
