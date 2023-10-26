#include <assert.h>

#include <stdio.h>

#include "utils.h"

uint8_t set_bit( uint8_t byte, uint8_t bit_num )
{
    assert( 0 <= bit_num && bit_num <= 7 );

    return byte | (uint8_t) (1U << bit_num);
}

uint8_t test_bit( uint8_t byte, uint8_t bit_num )
{
    assert( 0 <= bit_num && bit_num <= 7 );

    return byte & (uint8_t) (1U << bit_num);

}

int find_maximum_elem_width(const int *arr, size_t n_memb)
{
    assert(arr != NULL);

    int ans = 0;
    for (size_t ind = 0; ind < n_memb; ind++)
    {
        int res = find_num_width( arr[ind] );
        if (res > ans) ans = res;
    }
    return ans;
}

int find_num_width(int num)
{
    int res = 0;

    if (num < 0)
    {
        res++;
        num = -num;
    }

    while( num > 0 )
    {
        res++;
        num = num/10;
    }

    return res;
}

void put_n_chars(FILE *stream, size_t n, char c)
{
    while (n--)
    {
        putc(c, stream);
    }
}
