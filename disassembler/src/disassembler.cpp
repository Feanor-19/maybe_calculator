#include <stdio.h>

#include "disassembler.h"

void f(int *x)
{
    *x = 19;
}

#define F() ( f(&x), x )

int main()
{
    int a = 0;
    //a = (int x = 0, 0);
    printf("%d", a);
    return 0;
}
