#include <stdint.h>
#include "greatest/greatest.h"

#include "char_array.h"

#include "char_array_aligned.h"

#define SEPARATOR "|*|*|*|"


TEST test_char_array(void) {
    char_array *str = char_array_new();
    if (str == NULL) {
        FAIL();
    }
    char_array_cat(str, "Bürgermeister");
    char_array_cat_len(str, "straße", strlen("straße"));
    char_array_cat(str, "|");
    ASSERT_STR_EQ(str->a, "Bürgermeisterstraße|");

    char_array_cat_printf(str, "%d %s %.2f \t ", 1234, "onetwothreefour", 12.34);

    char *expected_output = "Bürgermeisterstraße|1234 onetwothreefour 12.34 \t ";
    ASSERT_STR_EQ(str->a, expected_output);

    char *a = char_array_to_string(str);
    ASSERT_STR_EQ(a, expected_output);

    free(a);

    str = char_array_new();

    char_array_add_joined(str, SEPARATOR, true, 3, "dictionaries" SEPARATOR, "foo", "bar");

    a = char_array_get_string(str);

    ASSERT_STR_EQ(a, "dictionaries|*|*|*|foo|*|*|*|bar");

    char_array_destroy(str);

    PASS();
}

TEST test_char_array_aligned(void) {
    char_array_aligned *str = char_array_aligned_new();
    if (str == NULL) {
        FAIL();
    }
    char_array_aligned_cat(str, "Bürgermeister");
    char_array_aligned_cat_len(str, "straße", strlen("straße"));
    char_array_aligned_cat(str, "|");

    ASSERT_STR_EQ(str->a, "Bürgermeisterstraße|");

    char_array_aligned_cat_printf(str, "%d %s %.2f \t ", 1234, "onetwothreefour", 12.34);

    char *expected_output = "Bürgermeisterstraße|1234 onetwothreefour 12.34 \t ";
    ASSERT_STR_EQ(str->a, expected_output);

    char *a = char_array_aligned_to_string(str);
    ASSERT_STR_EQ(a, expected_output);

    free(a);

    str = char_array_aligned_new();

    char_array_aligned_add_joined(str, SEPARATOR, true, 3, "dictionaries" SEPARATOR, "foo", "bar");

    a = char_array_aligned_get_string(str);
    ASSERT_STR_EQ(a, "dictionaries|*|*|*|foo|*|*|*|bar");

    char_array_aligned_destroy(str);

    PASS();
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int32_t main(int32_t argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */

    RUN_TEST(test_char_array);
    RUN_TEST(test_char_array_aligned);

    GREATEST_MAIN_END();        /* display results */
}
