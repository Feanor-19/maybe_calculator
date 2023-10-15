#include <assert.h>

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
