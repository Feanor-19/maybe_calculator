#include <stdio.h>
#include <ctype.h>

#include "disassembler.h"

int f(int x)
{
    static int j = 0;
    if (x >=0 )
    {
        j += x;
    }
    return j;
}

int main()
{
    printf("<%d>\n", f(0));
    printf("<%d>\n", f(1));
    printf("<%d>\n", f(2));
    printf("<%d>\n", f(0));
    printf("<%d>\n", f(19));
    return 0;
}
