#ifndef MYSTRING_H
#define MYSTRING_H

#include <stdlib.h>

size_t my_strlen(const char *s);

int my_puts(const char *s);

const char * my_strchr( const char * s, int symbol);

char * my_strchr( char * s, int symbol);

const char * my_strrchr(const char * s, int symbol);

char * my_strrchr(char * s, int symbol);

char * my_strcpy( char * destptr, const char * srcptr );

char * my_strncpy( char * destptr, const char * srcptr, size_t num );

char * my_strcat( char * destptr, const char * srcptr );

char * my_strncat( char * destptr, char * srcptr, size_t num );

char * my_fgets( char * s, int num, FILE * filestream );

char *my_strdup(const char *s);

size_t my_getline(char **lineptr, size_t *n, FILE *stream);

int my_strcmp( const char * str1, const char * str2 );

// @note way = 1 means straight way of comparing, way = -1 means reversed way of comparing
int my_strcmp(  const char *str1_begin,
                const char *str1_end,
                const char *str2_begin,
                const char *str2_end,
                int way = 1 );

const char * my_strstr( const char * str, const char * sub_str );

const char * my_strstr2( const char * str, const char * sub_str );

#endif /* MYSTRING_H */
