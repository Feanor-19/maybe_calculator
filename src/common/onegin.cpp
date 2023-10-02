#include "onegin.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>

#include "mystring.h"

FileBuf read_file_to_buf(const char *file_name, ErrorCodes *err)
{
    assert(file_name);

    FileBuf file_buf = {};

    off_t file_size = get_file_size(file_name);
    if (file_size == -1) {
        if (err) *err = ERROR_FILE_SIZE;
        return file_buf;
    }

    FILE *file_p = fopen(file_name, "r");
    if (file_p == NULL) {
        if (err) *err = ERROR_OPEN_FILE;
        return file_buf;
    }

    char *buf = (char *) calloc( file_size + 1, sizeof(char) );

    size_t buf_size = fread(buf, sizeof(char), file_size, file_p);
    if ( ferror(file_p) != 0 )
    {
        if (err) *err = ERROR_READ_FILE;
        free(buf);
        return file_buf;
    }

    file_buf.buf = buf;
    file_buf.buf_size = buf_size;

    fclose(file_p);
    file_p = NULL;

    return file_buf;
}

inline size_t count_lines_in_buf(FileBuf file_buf)
{
    size_t n_lines = 1;
    for (size_t ind = 0; ind < file_buf.buf_size; ind++)
    {
        if (file_buf.buf[ind] == '\n') n_lines++;
    }

    return n_lines;
}

Text parse_buf_to_text(FileBuf file_buf)
{

    size_t n_lines_in_buf = count_lines_in_buf(file_buf);

    char **line_array = (char **) calloc(n_lines_in_buf, sizeof(char *));

    line_array[0] = file_buf.buf;

    size_t line = 1;
    for (size_t ind = 0; ind < file_buf.buf_size - 1; ind++)
    {
        if ( file_buf.buf[ind] == '\n' )
        {
            file_buf.buf[ind] = '\0';
            line_array[line++] = &file_buf.buf[ind + 1];
        }
    }

    //printf("Info: curr_text_size = %u, free_place_in_line_arr = %u\n", curr_text_size, free_place_in_line_arr);

    Text text = {};
    text.line_array = line_array;
    text.nLines = line;

    return text;
}

void print_text(Text text, int do_print_addresses)
{
    print_text_to_stream(text, stdout, do_print_addresses);
}

void print_text_to_file(const char *file_name, Text text, int do_print_addresses, ErrorCodes *err)
{
    assert(file_name);

    FILE *file_p = fopen(file_name, "w");
    if (!file_p)
    {
        *err = ERROR_OPEN_FILE;
        return;
    }

    print_text_to_stream(text, file_p, do_print_addresses);

    fclose(file_p);
}

void print_text_to_stream( Text text, FILE *stream, int do_print_addresses )
{
    for (unsigned long ind = 0; ind < text.nLines; ind++ )
    {
        if (do_print_addresses)
        {
            fprintf(stream, "Line at address %p : <%s>\n", &text.line_array[ind], text.line_array[ind]);
        }
        else
        {
            fprintf(stream, "%s\n", text.line_array[ind]);
        }
    }
}

//! @brief Returns file's (of name 'file_name') size or -1 in case of error.
off_t get_file_size(const char *file_name)
{
    assert(file_name);

    struct stat st_buf = {};

    if ( stat(file_name, &st_buf) == -1) return -1;

    return st_buf.st_size;
}

void print_error_message( ErrorCodes err )
{
    switch (err)
        {
        case ERROR_OPEN_FILE:
            fprintf(stderr, "ERROR: Can't open file!\n");
            break;
        case ERROR_FILE_SIZE:
            fprintf(stderr, "ERROR: Can't get file size!\n");
            break;
        case ERROR_READ_FILE:
            fprintf(stderr, "ERROR: Can't read file!\n");
            break;
        case ERROR_NO:
            assert(0 && "Unreacheable line reached!");
            break;
        default:
            fprintf(stderr, "ERROR: Unknown error occurred!\n");
            break;
        }
}

char *read_line(FILE *stream)
{
    assert(stream);

    const size_t START_STR_SIZE = 16;

    char *str = (char *) calloc( START_STR_SIZE, sizeof(char) );
    if (str == NULL) return NULL;

    size_t curr_size = START_STR_SIZE;
    size_t read_bytes = my_getline(&str, &curr_size, stream);

    if ( str[read_bytes - 2] == '\n' ) str[read_bytes - 2] = '\0';

    return str;
}

void buf_free(FileBuf *buf)
{
    free(buf->buf);
    buf->buf_size = 0;
}

Text text_copy(Text source)
{
    assert(source.line_array);
    assert(source.nLines > 0);

    Text res = {};

    char **line_array = (char **) calloc(source.nLines, sizeof(char *));

    for (size_t ind = 0; ind < source.nLines; ind++)
    {
        line_array[ind] = source.line_array[ind];
    }

    res.line_array = line_array;
    res.nLines = source.nLines;

    return res;
}

void text_free(Text *text)
{
    free(text->line_array);
    text->nLines = 0;
}

inline void skip_not_alnum( const char ** const p_line )
{
    while ( **p_line != '\0' && !isalpha(**p_line) ) (*p_line)++;
}

int cmp_line_beginning( const void *v_line1, const void *v_line2 )
{
    assert(v_line1 != NULL);
    assert(v_line2 != NULL);

    const char *line1 = *((const char * const *) v_line1);
    const char *line2 = *((const char * const *) v_line2);

    skip_not_alnum(&line1);
    skip_not_alnum(&line2);

    return my_strcmp(line1, line2);
}

inline size_t skip_not_alnum_from_end( const char * line )
{
    assert(line);

    const char *p = line + my_strlen(line) - 1;

    while ( p != line && !isalnum(*p) ) p--;

    return p - line + 1;
}

int cmp_line_end( const void *v_line1, const void *v_line2 )
{
    assert(v_line1 != NULL);
    assert(v_line2 != NULL);

    const char *line1 = *((const char * const *) v_line1);
    const char *line2 = *((const char * const *) v_line2);

    size_t fact_len_1 = skip_not_alnum_from_end(line1);
    size_t fact_len_2 = skip_not_alnum_from_end(line2);

    //printf("### s1 = <%s> - <%c>, s2 = <%s> - <%c>\n", line1, *(line1 + fact_len_1), line2, *(line2 + fact_len_2));

    return my_strcmp(line1, line1 + fact_len_1, line2, line2 + fact_len_2, -1);
}
