#include <stdio.h>
#include <ctype.h>

#include "disassembler.h"

int main()
{
    char inp[10] = "";

    const char *str = "rax";
    if (sscanf(str, " [ %[^]] ] ", inp) != 1)
        printf("NO\n");
    else
        printf("%s\n", inp);

    return 0;
}
