/* char_array is a dynamic character array which is a vector of characters
with a few additional methods related to string manipulation.

The array pointer can be treated as a plain old C string for methods
expecting NUL-terminated char pointers, but using char_array_* functions
makes sure e.g. appends and concatenation are cheap and safe.
*/

#ifndef CHAR_ARRAY_H
#define CHAR_ARRAY_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>

#include "vector/vector.h"

VECTOR_INIT(char_array, char)

char_array *char_array_from_string(char *str);
char_array *char_array_from_string_no_copy(char *str, size_t n);

// Gets the underlying C string for a char_array
char *char_array_get_string(char_array *array);

// Frees the char_array and returns a standard NUL-terminated string
char *char_array_to_string(char_array *array);

// Can use strlen(array->a) but this is faster
size_t char_array_len(char_array *array);

// append_* methods do not NUL-terminate
void char_array_append(char_array *array, char *str);
void char_array_append_len(char_array *array, char *str, size_t len);
void char_array_append_reversed(char_array *array, char *str);
void char_array_append_reversed_len(char_array *array, char *str, size_t len);
// add NUL terminator to a char_array
void char_array_strip_nul_byte(char_array *array);
void char_array_terminate(char_array *array);

// add_* methods NUL-terminate without stripping NUL-byte
void char_array_add(char_array *array, char *str);
void char_array_add_len(char_array *array, char *str, size_t len);

// Similar to strcat but with dynamic resizing, guaranteed NUL-terminated
void char_array_cat(char_array *array, char *str);
void char_array_cat_len(char_array *array, char *str, size_t len);
void char_array_cat_reversed(char_array *array, char *str);
void char_array_cat_reversed_len(char_array *array, char *str, size_t len);

// Similar to cat methods but with printf args
void char_array_cat_vprintf(char_array *array, char *format, va_list args);
void char_array_cat_printf(char_array *array, char *format, ...);

// Mainly for paths or delimited strings
void char_array_add_vjoined(char_array *array, char *separator, bool strip_separator, size_t count, va_list args);
void char_array_add_joined(char_array *array, char *separator, bool strip_separator, size_t count, ...);
void char_array_cat_joined(char_array *array, char *separator, bool strip_separator, size_t count, ...);

#endif
