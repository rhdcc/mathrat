#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// TODO: A litte messy

typedef struct SV {
    char *str;
    size_t len;
} SV;

char *slurp_file(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if(file == NULL) {
        fprintf(stderr, "[ERROR]: Failed to open file '%s'.", filepath);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char *)malloc(length + 1);
    size_t bytes_read = fread(buffer, 1, length, file);
    if(bytes_read != length) {
        fprintf(stderr, "[ERROR]: Expected to read '%zd' bytes, actually read '%zd' bytes.", length, bytes_read);
        free(buffer);
        return NULL;
    }
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

char *match(char *pc, const char *str, int *good) {
    if(good != NULL) *good = 1;
    size_t len = strlen(str);
    for(size_t i = 0; i < len; ++i) {
        if(*pc == '\0') {
            if(good != NULL) *good = 1;
            break;
        }
        if(*pc != str[i]) {
            fprintf(stderr, "==> Match ERROR: Expected '%d', got '%d'\n", (int)str[i], (int)*pc);
            if(good != NULL) *good = 0;
            break;
        }
        pc++;
    }
    return pc;
}

char *read_integer(char *pc, SV *sv) {
    sv->str = pc;
    if(*pc == '-') {
        pc++;
    }
    while(is_digit(*pc)) {
        pc++;
    }
    sv->len = pc - sv->str;
    return pc;
}

char *skip_space(char *pc) {
    while(*pc == ' ' || *pc == '\t') pc++;
    return pc;
}

char *skip_newline(char *pc) {
    while(*pc == '\n' || *pc == '\r') pc++;
    return pc;
}

char *read_expected(char *pc, SV *expected) {
    int good;
    pc = match(pc, "Ex:", &good);
    assert(good && "[ERROR]: Could not match string 'Ex:'");
    pc = skip_space(pc);
    return read_integer(pc, expected);
}

char *sv_to_cstring(SV *sv) {
    char *buffer = (char *)malloc(sv->len + 1);
    memcpy(buffer, sv->str, sv->len);
    buffer[sv->len] = '\0';
    return buffer;
}

void write_operation(FILE *file, int type, SV *a, SV *b, SV *expected) {
    char *ca = sv_to_cstring(a);
    char *cb = sv_to_cstring(b);
    char *cexpected = sv_to_cstring(expected);
    fprintf(file, "    test(%d, \"%s\", %zd, \"%s\", %zd, \"%s\", %zd);\n", type, ca, a->len, cb, b->len, cexpected, expected->len);
    free(ca);
    free(cb);
    free(cexpected);
}

const char *TEST_FUNCTION = "\
void test(int type, char *sva, size_t size_a, char *svb, size_t size_b, char *sv_expected, size_t size_expected) {\n\
    printf(\"  ---  Test #%d  ---\\n\", test_counter);\n\
    printf(\"   Operation type: %s\\n\", type ? \"addition\" : \"subtraction\");\n\
    Integer a = integer_from_str(sva, size_a);\n\
    Integer b = integer_from_str(svb, size_b);\n\
    printf(\"=> \");\n\
    integer_debug_print(&a);\n\
    printf(\" %s \", type ? \"+\" : \"-\");\n\
    integer_debug_print(&b);\n\
    printf(\" = ??\\n\");\n\
\n\
    char *temp = (char *)malloc(size_expected + 1);\n\
    memcpy(temp, sv_expected, size_expected);\n\
    temp[size_expected] = \'\\0\';\n\
\n\
    Integer out = (type) ? integer_add(&a, &b) : integer_subtract(&a, &b); \n\
    printf(\"    Actual:   \");\n\
    integer_debug_print(&out);\n\
    printf(\"\\n    Expected: %s\\n\\n\", temp);\n\
\n\
    integer_free(&out);\n\
    integer_free(&a);\n\
    integer_free(&b);\n\
\n\
    test_counter++;\n\
    free(temp);\n\
}";

void make_tests(char *buffer) {
    FILE *file = fopen("./src/tests/tests.c", "w");
    fprintf(file, "/** AUTOMATICALLY GENERATED FILE **/\n");
    fprintf(file, "#include \"integer.h\"\n");
    fprintf(file, "#include <stdio.h>\n");
    fprintf(file, "#include <stdlib.h>\n\n");
    fprintf(file, "static int test_counter = 0;\n\n");

    fprintf(file, "%s\n\n", TEST_FUNCTION);

    fprintf(file, "void run_tests(void) {\n");
    char *pc = buffer;
    while(*pc != '\0') {
        switch(*pc++) {
            case '+': {
                pc = skip_space(pc); 
                SV a = {0};
                pc = read_integer(pc, &a);
                pc = skip_space(pc); 
                SV b = {0};
                pc = read_integer(pc, &b);
                pc = skip_newline(pc); 

                SV expected = {0};
                pc = read_expected(pc, &expected);
                pc = skip_newline(pc); 

                write_operation(file, 1, &a, &b, &expected);
            } break;

            case '-': {
                pc = skip_space(pc); 
                SV a = {0};
                pc = read_integer(pc, &a);
                pc = skip_space(pc); 
                SV b = {0};
                pc = read_integer(pc, &b);
                pc = skip_newline(pc); 

                SV expected = {0};
                pc = read_expected(pc, &expected);
                pc = skip_newline(pc); 

                write_operation(file, 0, &a, &b, &expected);
            } break;

            default: {
            }
        }
    }
    fprintf(file, "}\n");
    fprintf(file, "/** END OF GENERATED FILE **/\n");
    fclose(file);
}

int main(void) {
    char *buffer = slurp_file("./src/tests/tests.txt");
    make_tests(buffer);
    free(buffer);
}
