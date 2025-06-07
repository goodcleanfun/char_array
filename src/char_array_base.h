/* CHAR_ARRAY_NAME is a dynamic character array which is a vector of characters
with a few additional methods related to string manipulation.

The array pointer can be treated as a plain old C string for methods
expecting NUL-terminated char pointers, but using char_array_* functions
makes sure e.g. appends and concatenation are cheap and safe.
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef CHAR_ARRAY_ALIGNED
#define CHAR_ARRAY_NAME char_array_aligned
#define ARRAY_NAME char_array_aligned
#define ARRAY_TYPE char
#include "aligned_array/aligned_array.h"
#undef ARRAY_TYPE
#undef ARRAY_NAME
#else
#define CHAR_ARRAY_NAME char_array
#define ARRAY_NAME char_array
#define ARRAY_TYPE char
#include "array/array.h"
#undef ARRAY_TYPE
#undef ARRAY_NAME
#endif

#define CONCAT_(a, b) a ## b
#define CONCAT(a, b) CONCAT_(a, b)
#define CHAR_ARRAY_FUNC(func) CONCAT(CHAR_ARRAY_NAME, _##func)

// Gets the underlying C string for a char_array
static CHAR_ARRAY_NAME *CHAR_ARRAY_FUNC(from_string)(char *str) {
    size_t len = strlen(str);
    CHAR_ARRAY_NAME *array = CHAR_ARRAY_FUNC(new_size)(len+1);
    strcpy(array->a, str);
    array->n = len;
    return array;
}

static CHAR_ARRAY_NAME *CHAR_ARRAY_FUNC(from_string_no_copy)(char *str, size_t n) {
    CHAR_ARRAY_NAME *array = malloc(sizeof(CHAR_ARRAY_NAME));
    array->a = str;
    array->m = n;
    array->n = n;
    return array;
}

static inline void CHAR_ARRAY_FUNC(terminate)(CHAR_ARRAY_NAME *array) {
    CHAR_ARRAY_FUNC(push)(array, '\0');
}

static inline char *CHAR_ARRAY_FUNC(get_string)(CHAR_ARRAY_NAME *array) {
    if (array->n == 0 || array->a[array->n - 1] != '\0') {
        CHAR_ARRAY_FUNC(terminate)(array);
    }
    return array->a;
}

// Frees the CHAR_ARRAY_NAME and returns a standard NUL-terminated string
static inline char *CHAR_ARRAY_FUNC(to_string)(CHAR_ARRAY_NAME *array) {
    if (array->n == 0 || array->a[array->n - 1] != '\0') {
        CHAR_ARRAY_FUNC(terminate)(array);
    }
    char *a = array->a;
    free(array);
    return a;
}


// add NUL terminator to a CHAR_ARRAY_NAME
static inline void CHAR_ARRAY_FUNC(strip_nul_byte)(CHAR_ARRAY_NAME *array) {
    if (array->n > 0 && array->a[array->n - 1] == '\0') {
        array->a[array->n - 1] = '\0';
        array->n--;
    }
}

// Can use strlen(array->a) but this is faster
static inline size_t CHAR_ARRAY_FUNC(len)(CHAR_ARRAY_NAME *array) {
    if (array->n > 0 && array->a[array->n - 1] == '\0') {
        return array->n - 1;
    } else {
        return array->n;
    }
}

// append_* methods do not NUL-terminate
static inline void CHAR_ARRAY_FUNC(append)(CHAR_ARRAY_NAME *array, char *str) {
    while(*str) {
        CHAR_ARRAY_FUNC(push)(array, *str++);
    }    
}

static inline void CHAR_ARRAY_FUNC(append_len)(CHAR_ARRAY_NAME *array, char *str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        CHAR_ARRAY_FUNC(push)(array, *str++);
    }
}

// Similar to strcat but with dynamic resizing, guaranteed NUL-terminated
static inline void CHAR_ARRAY_FUNC(cat)(CHAR_ARRAY_NAME *array, char *str) {
    CHAR_ARRAY_FUNC(strip_nul_byte)(array);
    CHAR_ARRAY_FUNC(append)(array, str);
    CHAR_ARRAY_FUNC(terminate)(array);
}

static inline void CHAR_ARRAY_FUNC(cat_len)(CHAR_ARRAY_NAME *array, char *str, size_t len) {
    CHAR_ARRAY_FUNC(strip_nul_byte)(array);
    CHAR_ARRAY_FUNC(append_len)(array, str, len);
    CHAR_ARRAY_FUNC(terminate)(array);
}

// add_* methods NUL-terminate without stripping NUL-byte
static inline void CHAR_ARRAY_FUNC(add)(CHAR_ARRAY_NAME *array, char *str) {
    CHAR_ARRAY_FUNC(append)(array, str);
    CHAR_ARRAY_FUNC(terminate)(array);
}

static inline void CHAR_ARRAY_FUNC(add_len)(CHAR_ARRAY_NAME *array, char *str, size_t len) {
    CHAR_ARRAY_FUNC(append_len)(array, str, len);
    CHAR_ARRAY_FUNC(terminate)(array);
}

// Mainly for paths or delimited strings
static void CHAR_ARRAY_FUNC(add_vjoined)(CHAR_ARRAY_NAME *array, char *separator, bool strip_separator, size_t count, va_list args) {
    if (count <= 0) {
        return;        
    }

    size_t separator_len = strlen(separator);

    for (size_t i = 0; i < count - 1; i++) {
        char *arg = va_arg(args, char *);
        size_t len = strlen(arg);

        if (strip_separator && 
            ((separator_len == 1 && arg[len-1] == separator[0]) || 
            (len > separator_len && strncmp(arg + len - separator_len, separator, separator_len) == 0))) {
            len -= separator_len;
        }

        CHAR_ARRAY_FUNC(append_len)(array, arg, len);
        CHAR_ARRAY_FUNC(append)(array, separator);
    }

    char *arg = va_arg(args, char *);
    CHAR_ARRAY_FUNC(append)(array, arg);
    CHAR_ARRAY_FUNC(terminate)(array);

}

static inline void CHAR_ARRAY_FUNC(add_joined)(CHAR_ARRAY_NAME *array, char *separator, bool strip_separator, size_t count, ...) {
    va_list args;
    va_start(args, count);
    CHAR_ARRAY_FUNC(add_vjoined)(array, separator, strip_separator, count, args);
    va_end(args);
}

static inline void CHAR_ARRAY_FUNC(cat_joined)(CHAR_ARRAY_NAME *array, char *separator, bool strip_separator, size_t count, ...) {
    CHAR_ARRAY_FUNC(strip_nul_byte)(array);
    va_list args;
    va_start(args, count);
    CHAR_ARRAY_FUNC(add_vjoined)(array, separator, strip_separator, count, args);
    va_end(args);
}

// Similar to cat methods but with printf args
static void CHAR_ARRAY_FUNC(cat_vprintf)(CHAR_ARRAY_NAME *array, char *format, va_list args) {
    CHAR_ARRAY_FUNC(strip_nul_byte)(array);

    va_list cpy;

    char *buf;
    size_t buflen;

    size_t last_n = array->n;
    size_t size = array->m - array->n <= 2 ? array->m * 2 : array->m;

    while(1) {
        CHAR_ARRAY_FUNC(resize)(array, size);
        buf = array->a + last_n;
        buflen = size - last_n;
        if (buf == NULL) return;
        array->a[size - 2] = '\0';
        va_copy(cpy, args);
        vsnprintf(buf, buflen, format, cpy);
        if (array->a[size - 2] != '\0') {
            size *= 2;
            continue;
        } else {
            array->n += strlen(buf);
        }
        break;
    }
}

static void CHAR_ARRAY_FUNC(cat_printf)(CHAR_ARRAY_NAME *array, char *format, ...) {
    va_list args;
    va_start(args, format);
    CHAR_ARRAY_FUNC(cat_vprintf)(array, format, args);
    va_end(args);
}

#undef CONCAT_
#undef CONCAT
#undef CHAR_ARRAY_FUNC
#undef CHAR_ARRAY_NAME