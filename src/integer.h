#ifndef INTEGER_H
#define INTEGER_H

#include <stdint.h> // uint8_t
#include <stddef.h> // size_t

#define DIGIT_CHUNK_CAPACITY 5

typedef struct DigitChunk DigitChunk;
typedef struct DigitChunk {
    size_t count;
    DigitChunk *next;
    DigitChunk *prev;
    uint8_t memory[DIGIT_CHUNK_CAPACITY]; // NOTE: little endian
} DigitChunk;

typedef struct {
    int is_negative;
    size_t chunk_count;
    DigitChunk *head;
    DigitChunk *tail;
} Integer;

typedef enum Integer_Compare_Flag {
    INTEGER_CMP_SMALLER = 0,
    INTEGER_CMP_BIGGER  = 1,
    INTEGER_CMP_EQUAL   = 2
} Integer_Compare_Flag;

Integer integer_from_str(char *str, size_t string_size);
Integer integer_add_ex(Integer *a, Integer *b, int a_negative, int b_negative);
Integer integer_subtract_ex(Integer *a, Integer *b, int a_negative, int b_negative);
Integer integer_add(Integer *a, Integer *b);
Integer integer_subtract(Integer *a, Integer *b);
Integer_Compare_Flag integer_compare_ex(Integer *a, Integer *b, int a_negative, int b_negative);
Integer_Compare_Flag integer_compare(Integer *a, Integer *b);
void integer_debug_print(Integer *a);
void integer_free(Integer *a);

#endif // INTEGER_H
