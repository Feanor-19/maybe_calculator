#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

//! @brief Set's bit 'bit_num' number to 1 in the byte.
uint8_t set_bit( uint8_t byte, uint8_t bit_num );

//! @brief Test's bit 'bit_num' in the byte.
uint8_t test_bit( uint8_t byte, uint8_t bit_num );

int find_maximum_elem_width(const int *arr, size_t n_memb);

int find_num_width(int num);

void put_n_chars(FILE *stream, size_t n, char c);

#endif
