#include "integer.h"
#include "temp_buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

DigitChunk ZeroChunk = {0};

static DigitChunk *append_almost_zeroed_chunk(Integer *a) {
    if(a->head == NULL) {
        a->head = (DigitChunk *)malloc(sizeof(DigitChunk));
        memset(a->head, 0, sizeof(*a->head));
        a->tail = a->head;
        a->chunk_count += 1;
        return a->head;
    }
    DigitChunk *chunk = a->head;
    while(chunk->next != NULL) {
        chunk = chunk->next;
    }
    chunk->next = (DigitChunk *)malloc(sizeof(DigitChunk));
    memset(chunk->next, 0, sizeof(*chunk->next));
    chunk->next->prev = chunk; // NOTE: The 'almost' refers to this line and below
    a->tail = a->tail->next;
    a->chunk_count += 1;
    return chunk->next;
}

Integer integer_from_str(char *str, size_t string_size) { // TODO: Use 'append_almost_zeroed_chunk'
    // TODO: Handle -0...
    assert(string_size >= 1 && "[ERROR]: String size must be at least 1");
    Integer out = {0};
    size_t i_offset = 0;
    if(str[0] == '-') {
        assert(string_size > 1 && "[ERROR]: String of negative integer has no digits");
        out.is_negative = 1;
        str += 1;
        string_size -= 1;
    }
    DigitChunk *prev_chunk = NULL;
    while(i_offset < string_size) {
        DigitChunk *current_chunk = (DigitChunk *)malloc(sizeof(*current_chunk));
        current_chunk->prev = NULL;

        if(i_offset == 0) {
            out.head = current_chunk;
        }
        memset(current_chunk->memory, 0, DIGIT_CHUNK_CAPACITY);
        current_chunk->next = NULL;
        size_t i = i_offset;
        while(i < string_size && i - i_offset < DIGIT_CHUNK_CAPACITY) {
            uint8_t cb = str[string_size - 1 - i] - '0';
            memcpy(current_chunk->memory + (i - i_offset), &cb, sizeof(cb));
            i += 1;
        }
        assert(i - i_offset > 0 && "[ERROR]: Num bytes written to chunk must be at least 1");
        current_chunk->count = i - i_offset;
        if(prev_chunk) {
            current_chunk->prev = prev_chunk;
            prev_chunk->next = current_chunk;
        }
        prev_chunk = current_chunk;
        out.chunk_count += 1;
        i_offset = i;
    }
    out.tail = prev_chunk;
    return out;
}

void integer_debug_print(Integer *a) {
    if(a->is_negative) {
        printf("-");
    }
    DigitChunk *chunk = a->tail;
    for(size_t chunk_counter = 0; chunk_counter < a->chunk_count; ++chunk_counter) {
        assert(chunk != NULL && "[ERROR]: Integer DigitChunk is NULL!");
        for(size_t i = 0; i < chunk->count; ++i) {
            printf("%d", (unsigned int)chunk->memory[chunk->count - 1 - i]);
        }
        chunk = chunk->prev;
    }
}

Integer_Compare_Flag integer_compare_ex(Integer *a, Integer *b, int a_negative, int b_negative) {
    if(a_negative != b_negative) {
        return b_negative;
    }
    if(a->chunk_count != b->chunk_count) {
        return ((a->chunk_count > b->chunk_count) + a->is_negative) % 2;
    }
    DigitChunk *current_a_chunk = a->head;
    DigitChunk *current_b_chunk = b->head;
    for(int i = 0; i < a->chunk_count; ++i) {
        assert(current_a_chunk != NULL && "[ERROR]: Comparing NULL chunk!");
        assert(current_b_chunk != NULL && "[ERROR]: Comparing NULL chunk!");
        if(current_a_chunk->count != current_b_chunk->count) {
            return ((current_a_chunk->count > current_b_chunk->count) + a->is_negative) % 2;
        }
        for(size_t j = 0; j < current_a_chunk->count; ++j) {
            if(current_a_chunk->memory[j] != current_b_chunk->memory[j]) {
                return ((current_a_chunk->memory[j] > current_b_chunk->memory[j]) + a->is_negative) % 2;
            }
        }
        current_a_chunk = (current_a_chunk != NULL) ? current_a_chunk->next : NULL;
        current_b_chunk = (current_b_chunk != NULL) ? current_b_chunk->next : NULL;
    }
    return INTEGER_CMP_EQUAL;
}

Integer_Compare_Flag integer_compare(Integer *a, Integer *b) {
    return integer_compare_ex(a, b, a->is_negative, b->is_negative);
}

static size_t size_min(size_t a, size_t b) {
    if(a <= b) {
        return a;
    }
    return b;
}

static size_t size_max(size_t a, size_t b) {
    if(a >= b) {
        return a;
    }
    return b;
}

static void chunk_add(DigitChunk *a, DigitChunk *b, uint8_t old_carry) {
    uint8_t carry = old_carry;
    for(size_t i = 0; i < size_max(a->count, b->count); ++i) {
        uint8_t digit_a = (i >= a->count) ? 0 : a->memory[i];
        uint8_t digit_b = (i >= b->count) ? 0 : b->memory[i];
        uint8_t sum = carry + digit_a + digit_b;
        uint8_t out_digit = sum % 10;
        carry = (sum - out_digit) / 10;
        tmp_push(&out_digit, sizeof(out_digit));
    }
    if(carry != 0) {
        tmp_push(&carry, sizeof(carry));
    }
}

Integer integer_add(Integer *a, Integer *b) {
    return integer_add_ex(a, b, a->is_negative, b->is_negative);
}

Integer integer_add_ex(Integer *a, Integer *b, int a_negative, int b_negative) {
    assert(a->head != NULL && "[ERROR]: First integer operand for '+' has no digits!");
    assert(b->head != NULL && "[ERROR]: Second integer operand for '+' has no digits!");

    Integer out = {0};
    if(a_negative && !b_negative) {
        return integer_subtract_ex(b, a, 0, 0);
    } else if(!a_negative && b_negative) {
        return integer_subtract_ex(a, b, 0, 0);
    } else if(a->is_negative && b->is_negative) {
        out.is_negative = 1;
    }

    size_t min_chunk_count = size_min(a->chunk_count, b->chunk_count);
    size_t max_chunk_count = size_max(a->chunk_count, b->chunk_count);
    DigitChunk *current_a_chunk = a->head;
    DigitChunk *current_b_chunk = b->head;
    uint8_t carry = 0;
    for(size_t i = 0; i < max_chunk_count; ++i) {
        uint8_t *digits = (uint8_t *)tmp_start_scratch();
        if(i < min_chunk_count) {
            chunk_add(current_a_chunk, current_b_chunk, carry);
        } else {
            DigitChunk *chunk_to_add = (current_a_chunk != NULL) ? current_a_chunk : current_b_chunk;
            chunk_add(chunk_to_add, &ZeroChunk, carry);
        }
        size_t chunk_size = tmp_end_scratch();

        if(chunk_size > DIGIT_CHUNK_CAPACITY) {
            assert(chunk_size == DIGIT_CHUNK_CAPACITY + 1);
            carry = digits[chunk_size - 1];
        } else {
            carry = 0;
        }
        DigitChunk *appended_chunk = append_almost_zeroed_chunk(&out);
        appended_chunk->count = size_min(DIGIT_CHUNK_CAPACITY, chunk_size);
        appended_chunk->next = NULL;
        memcpy(appended_chunk->memory, digits, chunk_size * sizeof(*digits));

        if(current_a_chunk != NULL) current_a_chunk = current_a_chunk->next;
        if(current_b_chunk != NULL) current_b_chunk = current_b_chunk->next;
    }
    if(carry != 0) {
        DigitChunk *appended_chunk = append_almost_zeroed_chunk(&out);
        appended_chunk->next = NULL;
        appended_chunk->count = 1;
        memcpy(appended_chunk->memory, &carry, sizeof(carry));
    }
    return out;
}

static bool chunk_subtract(DigitChunk *a, DigitChunk *b, bool must_take, bool *is_zero, size_t *chunk_size) {
    *is_zero = true;
    size_t max_digit_count = size_max(a->count, b->count);
    size_t zero_count = 0;
    *chunk_size = 0;
    for(size_t i = 0; i < max_digit_count; ++i) {
        uint8_t a_digit = (i < a->count) ? a->memory[i] : 0;
        uint8_t b_digit = (i < b->count) ? b->memory[i] : 0;
        if(must_take) {
            // We need to take 1 (ONE!) from this difference...
            // add 1 to b_digit
            b_digit += 1;
        }

        must_take = false;
        uint8_t diff = a_digit - b_digit;
        if(a_digit < b_digit) {
            must_take = true;
            // Assume we have already taken 10 from the NEXT digit...
            diff = 10 - (b_digit - a_digit);
        }

        if(diff != 0) {
            *is_zero = false;
            *chunk_size += zero_count + 1;
            zero_count = 0;
        } else {
            zero_count += 1;
        }
        tmp_push(&diff, sizeof(diff));
    }
    return must_take;
}

Integer integer_subtract(Integer *a, Integer *b) {
    return integer_subtract_ex(a, b, a->is_negative, b->is_negative);
}

Integer integer_subtract_ex(Integer *a, Integer *b, int a_negative, int b_negative) {
    assert(a->head != NULL && "[ERROR]: First integer operand for '-' has no digits!");
    assert(b->head != NULL && "[ERROR]: Second integer operand for '-' has no digits!");

    Integer out = {0};
    Integer *first_sub_arg = a;
    Integer *second_sub_arg = b;
    if(integer_compare(b, a) == INTEGER_CMP_BIGGER) {
        first_sub_arg = b;
        second_sub_arg = a;
        out.is_negative = 1;
    }

    size_t max_chunk_count = size_max(a->chunk_count, b->chunk_count);
    bool must_take = false;

    DigitChunk *current_a_chunk = first_sub_arg->head;
    DigitChunk *current_b_chunk = second_sub_arg->head;
    size_t num_zero_chunks = 0;
    DigitChunk *last_chunk = NULL;
    size_t last_chunk_size;
    for(size_t i = 0; i < max_chunk_count; ++i) {
        DigitChunk *subtend_a = (current_a_chunk == NULL) ? &ZeroChunk : current_a_chunk;
        DigitChunk *subtend_b = (current_b_chunk == NULL) ? &ZeroChunk : current_b_chunk;

        uint8_t *scratch_buffer = (uint8_t *)tmp_start_scratch();
        bool is_zero = false;
        size_t chunk_size = 0;
        must_take = chunk_subtract(subtend_a, subtend_b, must_take, &is_zero, &chunk_size);
        assert(chunk_size <= DIGIT_CHUNK_CAPACITY && "[ERROR]: Chunk size after subtraction exceeds DIGIT_CHUNK_CAPACITY");
        size_t scratch_size = tmp_end_scratch();
        assert(scratch_size <= DIGIT_CHUNK_CAPACITY);

        if(is_zero) {
            num_zero_chunks += 1;
        } else {
            // append 'num_zero_chunks' zeroed chunks...
            for(size_t j = 0; j < num_zero_chunks; ++j) {
                DigitChunk *zchunk = append_almost_zeroed_chunk(&out);
                zchunk->count = DIGIT_CHUNK_CAPACITY;
            }

            // do the copy from the temp buffer into the DigitChunk...
            DigitChunk *appended_chunk = append_almost_zeroed_chunk(&out);
            appended_chunk->count = scratch_size;
            memcpy(appended_chunk->memory, scratch_buffer, scratch_size);

            // Reset num_zero_chunks
            num_zero_chunks = 0;
            last_chunk = appended_chunk;
            last_chunk_size = chunk_size;
        }

        if(current_a_chunk != NULL) current_a_chunk = current_a_chunk->next;
        if(current_b_chunk != NULL) current_b_chunk = current_b_chunk->next;
    }

    if(last_chunk == NULL) {
        last_chunk = append_almost_zeroed_chunk(&out);
        last_chunk->count = 1;
        out.is_negative = 0;
    } else {
        last_chunk->count = last_chunk_size;
    }

    assert(must_take == false && "[ERROR]: Result of subtraction is negative!");
    return out;
}

static uint8_t chunk_digit_mul(DigitChunk *a, uint8_t digit, uint8_t old_carry) {
    uint8_t carry = old_carry;
    for(size_t i = 0; i < a->count; ++i) {
        uint8_t product = digit * a->memory[i] + carry;
        uint8_t out_digit = product % 10;
        carry = (product - out_digit) / 10;
        tmp_push(&out_digit, sizeof(out_digit));
    }
    return carry;
}

static Integer int_digit_mul(Integer *a, uint8_t digit, size_t pad) {
    Integer out = {0};
    for(size_t i = 0; i < pad / DIGIT_CHUNK_CAPACITY; ++i) {
        DigitChunk *zchunk = append_almost_zeroed_chunk(&out);
        zchunk->count = DIGIT_CHUNK_CAPACITY;
    }
    DigitChunk *zchunk = append_almost_zeroed_chunk(&out);
    zchunk->count = pad % DIGIT_CHUNK_CAPACITY;
    uint8_t carry = 0;
    size_t count_of_last = out.tail->count;
    for(DigitChunk *c = a->head; c != NULL; c = c->next) {
        uint8_t *scratch = tmp_start_scratch();
        carry = chunk_digit_mul(c, digit, carry);
        size_t scratch_size = tmp_end_scratch();
        assert(scratch_size <= DIGIT_CHUNK_CAPACITY && "Scratch buffer size exceeds max chunk size");
        if(count_of_last < DIGIT_CHUNK_CAPACITY) { // split, allocating new chunk may not be necessary
            size_t available_space = DIGIT_CHUNK_CAPACITY - count_of_last;
            if(scratch_size <= available_space) { // no allocation needed
                memcpy(out.tail->memory + count_of_last, scratch, scratch_size);
                out.tail->count += scratch_size;
            } else { // allocation needed
                out.tail->count = DIGIT_CHUNK_CAPACITY;
                memcpy(out.tail->memory + count_of_last, scratch, available_space);
                DigitChunk *new_chunk = append_almost_zeroed_chunk(&out);
                new_chunk->count = scratch_size - available_space;
                memcpy(new_chunk->memory, scratch + available_space, new_chunk->count);
            }
        } else { // no split needed, allocation is necessary
            DigitChunk *chunk_product = append_almost_zeroed_chunk(&out);
            chunk_product->count = scratch_size;
            memcpy(chunk_product->memory, scratch, chunk_product->count);
        }
        count_of_last = out.tail->count;
    }
    if(carry != 0) {
        if(out.tail->count < DIGIT_CHUNK_CAPACITY) {
            out.tail->memory[out.tail->count] = carry;
            out.tail->count += 1;
        } else {
            DigitChunk *last_chunk = append_almost_zeroed_chunk(&out);
            last_chunk->count = 1;
            last_chunk->memory[0] = carry;
        }
    }
    return out;
}

Integer int_int_mul(Integer *a, Integer *b) {
    assert(b->chunk_count >= 1);
    Integer zinteger = {0};
    DigitChunk *zchunk = append_almost_zeroed_chunk(&zinteger);
    zchunk->count = 1;
    if(
        (a->chunk_count == 1 && a->head->count == 1 && a->head->memory[0] == 0) ||
        (b->chunk_count == 1 && b->head->count == 1 && b->head->memory[0] == 0)
    ) {
        return zinteger;
    }
    DigitChunk *b_chunk = b->head;
    Integer summands[2] = {0};
    summands[0] = zinteger;
    size_t b_digit_count = b->tail->count + (b->chunk_count - 1) * DIGIT_CHUNK_CAPACITY;
    size_t local_index = 0;
    for(size_t b_digit_index = 0; b_digit_index < b_digit_count; ++b_digit_index) {
        if(local_index >= DIGIT_CHUNK_CAPACITY) {
            local_index = 0;
            b_chunk = b_chunk->next;
        }
        uint8_t digit = b_chunk->memory[local_index];
        summands[1] = int_digit_mul(a, digit, b_digit_index);
        Integer sum = integer_add(&(summands[0]), &(summands[1]));
        integer_free(&(summands[0]));
        integer_free(&(summands[1]));
        summands[0] = sum;
        local_index += 1;
    }
    if(a->is_negative == b->is_negative) {
        summands[0].is_negative = 0;
    } else {
        summands[0].is_negative = 1;
    }
    return summands[0];
}

void integer_free(Integer *a) {
    DigitChunk *node = a->head;
    while(node != NULL) {
        DigitChunk *to_free = node;
        node = node->next;
        free(to_free);
    }
    a->head = NULL;
    a->tail = NULL;
}
