#include <stdio.h>
#include "char_array.h"
#include "utf8/utf8.h"
#include "utf8proc/utf8proc.h"

char_array *char_array_from_string(char *str) {
    size_t len = strlen(str);
    char_array *array = char_array_new_size(len+1);
    strcpy(array->a, str);
    array->n = len;
    return array;
}

char_array *char_array_from_string_no_copy(char *str, size_t n) {
    char_array *array = malloc(sizeof(char_array));
    array->a = str;
    array->m = n;
    array->n = n;
    return array;
}

inline char *char_array_get_string(char_array *array) {
    if (array->n == 0 || array->a[array->n - 1] != '\0') {
        char_array_terminate(array);
    }
    return array->a;
}

inline char *char_array_to_string(char_array *array) {
    if (array->n == 0 || array->a[array->n - 1] != '\0') {
        char_array_terminate(array);
    }
    char *a = array->a;
    free(array);
    return a;
}


inline void char_array_strip_nul_byte(char_array *array) {
    if (array->n > 0 && array->a[array->n - 1] == '\0') {
        array->a[array->n - 1] = '\0';
        array->n--;
    }
}

inline size_t char_array_len(char_array *array) {
    if (array->n > 0 && array->a[array->n - 1] == '\0') {
        return array->n - 1;
    } else {
        return array->n;
    }
}

inline void char_array_append(char_array *array, char *str) {
    while(*str) {
        char_array_push(array, *str++);
    }    
}

inline void char_array_append_len(char_array *array, char *str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char_array_push(array, *str++);
    }
}

inline void char_array_append_reversed_len(char_array *array, char *str, size_t len) {
    int32_t unich;
    ssize_t char_len;

    size_t idx = len;
    uint8_t *ptr = (uint8_t *)str;

    while(idx > 0) {
        char_len = utf8proc_iterate_reversed(ptr, idx, &unich);
        if (char_len <= 0 || unich == 0) break;
        if (!(utf8proc_codepoint_valid(unich))) break;

        idx -= char_len;
        char_array_append_len(array, str + idx, char_len);
    }
}

inline void char_array_append_reversed(char_array *array, char *str) {
    size_t len = strlen(str);
    char_array_append_reversed_len(array, str, len);
}

inline void char_array_terminate(char_array *array) {
    char_array_push(array, '\0');
}

inline void char_array_cat(char_array *array, char *str) {
    char_array_strip_nul_byte(array);
    char_array_append(array, str);
    char_array_terminate(array);
}

inline void char_array_cat_len(char_array *array, char *str, size_t len) {
    char_array_strip_nul_byte(array);
    char_array_append_len(array, str, len);
    char_array_terminate(array);
}


inline void char_array_cat_reversed(char_array *array, char *str) {
    char_array_strip_nul_byte(array);
    char_array_append_reversed(array, str);
    char_array_terminate(array);
}


inline void char_array_cat_reversed_len(char_array *array, char *str, size_t len) {
    char_array_strip_nul_byte(array);
    char_array_append_reversed_len(array, str, len);
    char_array_terminate(array);
}

inline void char_array_add(char_array *array, char *str) {
    char_array_append(array, str);
    char_array_terminate(array);
}

inline void char_array_add_len(char_array *array, char *str, size_t len) {
    char_array_append_len(array, str, len);
    char_array_terminate(array);
}


void char_array_add_vjoined(char_array *array, char *separator, bool strip_separator, size_t count, va_list args) {
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

        char_array_append_len(array, arg, len);
        char_array_append(array, separator);
    }

    char *arg = va_arg(args, char *);
    char_array_append(array, arg);
    char_array_terminate(array);

}

inline void char_array_add_joined(char_array *array, char *separator, bool strip_separator, size_t count, ...) {
    va_list args;
    va_start(args, count);
    char_array_add_vjoined(array, separator, strip_separator, count, args);
    va_end(args);
}

inline void char_array_cat_joined(char_array *array, char *separator, bool strip_separator, size_t count, ...) {
    char_array_strip_nul_byte(array);
    va_list args;
    va_start(args, count);
    char_array_add_vjoined(array, separator, strip_separator, count, args);
    va_end(args);
}

void char_array_cat_vprintf(char_array *array, char *format, va_list args) {
    char_array_strip_nul_byte(array);

    va_list cpy;

    char *buf;
    size_t buflen;

    size_t last_n = array->n;
    size_t size = array->m - array->n <= 2 ? array->m * 2 : array->m;

    while(1) {
        char_array_resize(array, size);
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

void char_array_cat_printf(char_array *array, char *format, ...) {
    va_list args;
    va_start(args, format);
    char_array_cat_vprintf(array, format, args);
    va_end(args);
}
