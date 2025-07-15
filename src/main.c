#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "integer.h"
#include "temp_buffer.h"

#include "tests/tests.c"

int main(void) {
    // run_tests();
    char *as = "69";
    char *bs = "420";
    Integer a = integer_from_str(as, strlen(as));
    Integer b = integer_from_str(bs, strlen(bs));
    Integer c = int_int_mul(&a, &b);
    integer_debug_print(&c);
    printf("\n");
    return 0;
}

