#ifndef PARSER_CMD_ARGS_H
#define PARSER_CMD_ARGS_H

#include <assert.h>
#include <string.h>
#include <stdio.h>

//! @brief Maximum length for additional argument (see struct CmdLineFlags)
const size_t MAX_ADD_ARG_LEN = 101;

//! @brief Represents one command line flag.
//! @note Some flags require some string to be passed right after them. This string is stored as additional argument.
struct CmdLineFlag
{
    const char *flag;               //!< The flag as a string, e.g. "-h"
    int state;                      //!< State of flag.
    int needs_add_arg;              //!< Must be set 1 if flag needs additional argument.
    char add_arg[MAX_ADD_ARG_LEN];  //!< Additional argument.
};

/*!
    @brief Parses command line arguments.
    @param [in] argc Number of all arguments passed, like in main(int argc, const char *argv[]).
    @param [in] argv Passed arguments as array of strings, like in main(int argc, const char *argv[]).
    @param [in] n_flags Number of supported flags.
    @param [out] flags Array of structs CmdLineFlag. Its length must be equal to n_flags.
    @return 0 if no errors occurred, number of read unsupported flags otherwise.
*/
int parse_cmd_args(int argc, const char * argv[], size_t n_flags, CmdLineFlag flags[]);

/*!
    @brief Finds CmdLineFlag in flags[] by flag as string and returns pointer to it.
    @param [in] n_flags Number of flags in flags[].
    @param [in] flags[] Array of CmdLineFlag, filled by parse_cmd_args().
    @param [in] flag Flag as a string, e.g. "-h".
*/
CmdLineFlag* extract(size_t n_flags, CmdLineFlag flags[], const char* flag);

#endif
