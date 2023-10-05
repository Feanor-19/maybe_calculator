#include <stdio.h>

#include "config_asm.h"

#include "assembler.h"

int main(int argc, const char *argv[])
{
    Config cfg = get_config(argc, argv);
    if (cfg.error)
    {
        print_cfg_error_message(stderr, cfg.error);
        return ASM_ERROR_GET_IN_OUT_FILES_NAMES;
    }

    print_config(stdout, cfg);

    const char *file_in_name = cfg.data_source;
    const char *file_out_name = cfg.output_destination;


}
