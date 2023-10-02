#include <assert.h>
#include <stdio.h>

#include "mystring.h"

size_t my_strlen(const char *s)
{
    assert(s != NULL);
    int len = 0;
    while (*(s++) != '\0') len++;
    return len;
}

int my_puts(const char *s)
{
    assert(s != NULL);
    int cnt = 0;
    while (*s != '\0')
    {
        if (putc(*s, stdout) == EOF)
        {
            return EOF;
        }
        s++;
        cnt++;
    }
    putc('\n', stdout);
    return cnt;
}

const char * my_strchr( const char * s, int symbol)
{
    assert(s != NULL);
    while ( *s != symbol && *s != '\0' ) s++;

    if (*s == '\0')
    {
        if (symbol == '\0')
        {
            return s;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return s;
    }
}

char * my_strchr( char * s, int symbol)
{
    assert(s != NULL);
    while ( *s != symbol && *s != '\0' ) s++;

    if (*s == '\0')
    {
        if (symbol == '\0')
        {
            return s;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return s;
    }
}

const char * my_strrchr(const char * s, int symbol)
{
    assert(s != NULL);

    const char *result = NULL;
    while ( *s != '\0' )
    {
        if ( *s == (char) symbol ) result = s;
        s++;
    }

    if (symbol == '\0')
    {
        return s;
    }
    else
    {
        return result;
    }
}

char * my_strrchr(char * s, int symbol)
{
    assert(s != NULL);

    char *result = NULL;
    while ( *s != '\0' )
    {
        if ( *s == (char) symbol ) result = s;
        s++;
    }

    if (symbol == '\0')
    {
        return s;
    }
    else
    {
        return result;
    }
}

char * my_strcpy( char * destptr, const char * srcptr )
{
    assert(destptr != NULL);
    assert(srcptr != NULL);
    assert(destptr != srcptr);

    while (*srcptr != '\0') *(destptr++) = *(srcptr++);
    *destptr = '\0';

    return destptr;
}

char * my_strncpy( char * destptr, const char * srcptr, size_t num )
{
    assert(destptr != NULL);
    assert(srcptr != NULL);
    assert(destptr != srcptr);

    char * res = destptr;
    while (num-- && *srcptr != '\0' && *destptr != '\0')
    {
        *(destptr++) = *(srcptr++);
    }

    *destptr = '\0';

    if ( *srcptr == '\0' && num > 0)
    {
        *destptr = '\0';
        return res;
    }

    return res;
}

char * my_strcat( char * destptr, const char * srcptr )
{
    assert(destptr != NULL);
    assert(srcptr != NULL);
    assert(destptr != srcptr);

    char *res = destptr;

    destptr = my_strchr(destptr, '\0');

    while (*srcptr != '\0')
    {
        *(destptr++) = *(srcptr++);
    }

    *destptr = '\0';

    return res;
}

char * my_strncat( char * destptr, char * srcptr, size_t num )
{
    assert(destptr != NULL);
    assert(srcptr != NULL);
    assert(destptr != srcptr);

    char *res = destptr;

    destptr = my_strchr(destptr, '\0');

    while (*srcptr != '\0' && num--)
    {
        *(destptr++) = *(srcptr++);
    }

    *destptr = '\0';

    return res;
}

char * my_fgets( char * s, int num, FILE * filestream )
{
    assert(s != NULL);
    assert(filestream != NULL);
    assert(num >= 0);

    int c = '\0';

    char *res = s;

    while ( (c = getc(filestream) ) != EOF && c != '\n' && num--)
    {
        *(s++) = (char) c;
    }

    if (num == 0)
    {
        *s = '\0';
        return res;
    }

    if (c == '\n')
    {
        *s = '\n';
        s++;
    }

    *s = '\0';

    if (c == EOF && res == s)
    {
        return NULL;
    }

    return res;
}

char *my_strdup(const char *s)
{
    assert(s != NULL);

    size_t s_len = my_strlen(s);

    char *new_s = (char*) calloc( s_len + 1, sizeof(char) );

    if (new_s == NULL) return NULL;

    char *p = new_s;
    while (*s != '\0')
    {
        *(p++) = *(s++);
    }

    *p = '\0';

    return new_s;
}

size_t my_getline(char **lineptr, size_t *n, FILE *stream)
{
    assert(lineptr != NULL);
    assert(*lineptr != NULL);
    assert(n != NULL);
    assert(*n > 3);

    const size_t MEM_MULTIPLIER = 2;

    int c = '\0';
    size_t read_bytes = 0; //количество прочитанных "байтов" (символов)
    size_t free_mem = *n; //количество оставшейся свободной памяти в "байтах"
    char *p_char = *lineptr; //указатель на рассматриваемый сейчас символ в строке

    while ( (c = getc(stream)) != EOF && c != '\n' )
    {
        *(p_char++) = (char) c;
        read_bytes++;
        free_mem--;

        if (free_mem == 2) //нужно оставлять достаточно памяти для '\n' и '\0'
        {
            free_mem = MEM_MULTIPLIER * (*n) - (*n); // выделяем больше памяти
            *n = MEM_MULTIPLIER * (*n);
            char *new_mem = (char *) calloc( *n, sizeof(char) );

            // копируем старую строку на новое место
            my_strcpy(new_mem, *lineptr);

            // освобождаем старую память
            free(*lineptr);

            // теперь lineptr хранит указатель на указатель на новую память
            *lineptr = new_mem;

            // теперь надо p_char перенаправить на первый свободный символ новой памяти
            p_char = *lineptr + read_bytes;
        }
    }

    //на данный момент есть как минимум два свободных байта памяти

    if (c == '\n')
    {
        *(p_char++) = '\n';
        read_bytes++;

        *p_char = '\0';
        read_bytes++; // по ТЗ нуль-символ тоже должен учитываться

        return read_bytes;
    }
    else if (c == EOF)
    {
        if (read_bytes == 0) return EOF;

        // По ТЗ если встретили EOF, но что-то прочитали, нужно вернуть без символа новой строки
        *p_char = '\0';
        read_bytes++;
        return read_bytes;
    }

    assert(0 && "Unreachable line!");
    return 0;
}

int my_strcmp( const char * str1, const char * str2 )
{
    assert(str1 != NULL);
    assert(str2 != NULL);

    while (*str1 != '\0' && *str2 != '\0' && *str1 == *str2)
    {
        str1++;
        str2++;
    }

    return ( (*str1 > *str2) ? 1 : -( *str1 < *str2 ) );
}

int my_strcmp(  const char *str1_begin,
                const char *str1_end,
                const char *str2_begin,
                const char *str2_end,
                int way )
{
    assert(str1_begin != NULL);
    assert(str1_end != NULL);
    assert(str2_begin != NULL);
    assert(str2_end != NULL);

    if (str1_begin == str1_end && str2_begin == str2_end) return 0;
    if (str1_begin == str1_end) return -1;
    if (str2_begin == str2_end) return 1;

    if ( !(way == 1 || way == -1) ) way = 1;

    const char *str1_p = NULL;
    const char *str2_p = NULL;

    if (way == 1)
    {
        str1_p = str1_begin;
        str2_p = str2_begin;

        while (str1_p != str1_end && str2_p != str2_end && *str1_p == *str2_p)
        {
            str1_p += 1;
            str2_p += 1;
        }

        //return ( (*str1_p > *str2_p) ? 1 : -( *str1_p < *str2_p ) );
    }
    else
    {
        str1_p = str1_end - 1;
        str2_p = str2_end - 1;

        while (str1_p > str1_begin && str2_p > str2_begin && *str1_p == *str2_p)
        {
            str1_p -= 1;
            str2_p -= 1;
        }

        //printf("@@@ %d\n", str2_p == str2_begin);

        if (str1_p >  str1_begin && str2_p == str2_begin) return 1;
        if (str1_p == str1_begin && str2_p  > str2_begin) return -1;

        //return ( (*str1_p > *str2_p) ? 1 : -( *str1_p < *str2_p ) );

    }

    return ( (*str1_p > *str2_p) ? 1 : -( *str1_p < *str2_p ) );

    //assert(0 && "Unreachable line!");
    //return 0;
}

const char * my_strstr( const char * str, const char * sub_str )
{
    assert(str != NULL);
    assert(sub_str != NULL);

    size_t sub_str_len = my_strlen(sub_str);

    while (*str != '\0')
    {
        if ( my_strcmp( str, str + sub_str_len - 1, sub_str, sub_str + sub_str_len - 1 ) == 0 )
        {
            return str;
        }
        str++;
    }

    return NULL;
}

/*!
    @brief Finds which symbols are presented in passe string.
    @param [in] arrSize Size of is_symb_in_substr[].
    @param [in/out] is_symb_in_substr[] Array, which index is symbol and value by index is 1 or 0,
    dependig whether this symbol is presented in string or not. Array must be initialized with zeroes.
*/
/*
static void symbols_in_str( int arrSize, int is_symb_in_substr[], const char* str )
{
    while (*str != '\0')
    {
        if ( *str < arrSize ) is_symb_in_substr[ (size_t) *str ]++;
        str++;
    }
}
*/

const char * my_strstr2( const char * str, const char * sub_str )
{
    assert(str != NULL);
    assert(sub_str != NULL);

    const size_t NSYMBOLS = 256;

    size_t sub_str_len = my_strlen(sub_str);
    const char *str_end = my_strchr(str, '\0');

    int prep_rfind[NSYMBOLS] = {0};

    for (size_t c = 0; c < NSYMBOLS; c++)
    {
        const char *rchr = my_strrchr( sub_str, (int) c );
        if (rchr != NULL) prep_rfind[ (size_t) c ] = (int) (sub_str + sub_str_len - rchr);
    }

    while (*str != '\0')
    {
        //прыжок
        if ( (str + sub_str_len - 1) < str_end )
        {
            assert( 0 <= *(str + sub_str_len - 1) && (size_t) *(str + sub_str_len - 1) < NSYMBOLS );

            size_t suffix_len = prep_rfind[ (size_t) *(str + sub_str_len - 1) ];

            size_t jmp_len = 0;

            if (suffix_len == 0) jmp_len = sub_str_len;
            else jmp_len = suffix_len - 1;

            str += jmp_len;
        }
        else
        {
            return NULL;
        }

        if ((str + sub_str_len - 1) >= str_end) return NULL;

        if ( my_strcmp( str, str + sub_str_len - 1, sub_str, sub_str + sub_str_len - 1 ) == 0 )
        {
            return str;
        }
        str++;
    }

    return NULL;
}
