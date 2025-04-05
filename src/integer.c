#include "integer.h"
#include "temp_buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

DigitChunk ZeroChunk = {0};

static DigitChunk *append_zeroed_chunk(Integer *a) {
	if(a->head == NULL) {
		a->head = (DigitChunk *)malloc(sizeof(DigitChunk));
		memset(a->head, 0, sizeof(*a->head));
		a->chunk_count += 1;
		return a->head;
	}
	DigitChunk *chunk = a->head;
	while(chunk->next != NULL) {
		chunk = chunk->next;
	}
	chunk->next = (DigitChunk *)malloc(sizeof(DigitChunk));
	memset(chunk->next, 0, sizeof(*chunk->next));
	a->chunk_count += 1;
	return chunk->next;
}

Integer integer_from_str(char *str, size_t string_size) {
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
			prev_chunk->next = current_chunk;
		}
		prev_chunk = current_chunk;
		out.chunk_count += 1;
		i_offset = i;
	}
	return out;
}

void integer_debug_print(Integer *a) {
	DigitChunk *chunk = a->head;
	for(size_t chunk_counter = 0; chunk_counter < a->chunk_count; ++chunk_counter) {
		assert(chunk != NULL && "[ERROR]: Integer DigitChunk is NULL!");
		for(size_t i = 0; i < chunk->count; ++i) {
			printf("%d", (unsigned int)chunk->memory[i]);
		}
		chunk = chunk->next;
		printf(" ");
	}
	if(a->is_negative) {
		printf("(-)");
	}
	printf("\n");
}

Integer_Compare_Flag integer_compare(Integer *a, Integer *b) {
	if(a->is_negative != b->is_negative) {
		return b->is_negative;
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

static void chunk_add(DigitChunk *a, DigitChunk *b, uint8_t old_carry) {
	uint8_t carry = old_carry;
	for(size_t i = 0; i < max(a->count, b->count); ++i) {
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
	assert(a->head != NULL && "[ERROR]: First integer operand for '+' has no digits!");
	assert(b->head != NULL && "[ERROR]: Second integer operand for '+' has no digits!");

	Integer out = {0};
	if(a->is_negative != b->is_negative) {
		return integer_subtract(a, b);
	} else if(a->is_negative == 1 && b->is_negative == 1) {
		out.is_negative = 1;
	}

	size_t min_chunk_count = min(a->chunk_count, b->chunk_count);
	size_t max_chunk_count = max(a->chunk_count, b->chunk_count);
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
		DigitChunk *appended_chunk = append_zeroed_chunk(&out);
		appended_chunk->count = min(DIGIT_CHUNK_CAPACITY, chunk_size);
		appended_chunk->next = NULL;
		memcpy(appended_chunk->memory, digits, chunk_size * sizeof(*digits));

		if(current_a_chunk != NULL) current_a_chunk = current_a_chunk->next;
		if(current_b_chunk != NULL) current_b_chunk = current_b_chunk->next;
	}
	if(carry != 0) {
		DigitChunk *appended_chunk = append_zeroed_chunk(&out);
		appended_chunk->next = NULL;
		appended_chunk->count = 1;
		memcpy(appended_chunk->memory, &carry, sizeof(carry));
	}
	return out;
}

static bool chunk_subtract(DigitChunk *a, DigitChunk *b, bool must_take, bool *is_zero, size_t *chunk_size) {
	*is_zero = true;
	size_t max_digit_count = max(a->count, b->count);
	size_t zero_count = 0;
	*chunk_size = 0;
	for(size_t i = 0; i < max_digit_count; ++i) {
		uint8_t a_digit = (i < a->count) ? a->memory[i] : 0;
		uint8_t b_digit = (i < b->count) ? b->memory[i] : 0;
		if(must_take) {
			// We need to take 1 (ONE!) from this difference...
			// either add 1 to b_digit, OR subtract 1 from a_digit (not doing this...)
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

	size_t max_chunk_count = max(a->chunk_count, b->chunk_count);
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
				DigitChunk *zchunk = append_zeroed_chunk(&out);
				zchunk->count = DIGIT_CHUNK_CAPACITY;
			}

			// do the copy from the temp buffer into the DigitChunk...
			DigitChunk *appended_chunk = append_zeroed_chunk(&out);
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
		last_chunk = append_zeroed_chunk(&out);
		last_chunk->count = 1;
		out.is_negative = 0;
	} else {
		last_chunk->count = last_chunk_size;
	}

	assert(must_take == false && "[ERROR]: Result of subtraction is negative!");
	return out;
}
