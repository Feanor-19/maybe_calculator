#ifndef ONEGIN_H
#define ONEGIN_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

enum ErrorCodes
{
    ERROR_NO        = 0,
    ERROR_FILE_SIZE = 1,
    ERROR_OPEN_FILE = 2,
    ERROR_READ_FILE = 3,
};

struct Text
{
    char **line_array = NULL;   //< массив адресов строк
    unsigned long nLines = 0;   //< кол-во строк в файле, aka размер text[]
};

struct FileBuf
{
    char *buf = NULL;
    size_t buf_size = 0;
};

//---

FileBuf read_file_to_buf(const char *file_name, ErrorCodes *err = NULL);

Text parse_buf_to_text(FileBuf file_buf);

void print_text(Text text, int do_print_addresses);

void print_text_to_file(const char *file_name, Text text, int do_print_addresses, ErrorCodes *err);

void print_text_to_stream( Text text, FILE *stream, int do_print_addresses );

off_t get_file_size(const char *file_name);

void print_error_message( ErrorCodes err );

//! @brief Reads one line from stream WITHOUT '\n'!
char *read_line(FILE *stream);

void buf_free(FileBuf *buf);

Text text_copy(Text source);

void text_free(Text *text);

int cmp_line_beginning( const void *v_line1, const void *v_line2 );

int cmp_line_end( const void *v_line1, const void *v_line2 );

#endif
