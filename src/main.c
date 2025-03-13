#include <stdio.h>

#include <string.h>

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define DIGIT_CHUNK_CAPACITY 5

typedef struct DigitChunk DigitChunk;
typedef struct DigitChunk {
	size_t count;
	DigitChunk *next;
	uint8_t memory[DIGIT_CHUNK_CAPACITY]; // NOTE: little endian
} DigitChunk;

DigitChunk ZeroChunk = {0};

typedef struct {
	size_t chunk_count;
	DigitChunk *head;
} Integer;

Integer integer_from_str(char *str, size_t string_size) {
	assert(string_size >= 1 && "[ERROR]: String size must be greater than 1");
	Integer out = {0};
	size_t i_offset = 0;
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
	printf("\n");
}

#define TEMP_BUFFER_CAPACITY 1024
uint8_t temp_buffer[TEMP_BUFFER_CAPACITY];
size_t tmp_buffer_count = 0;
size_t tmp_scratch_pos = 0;

void *tmp_push(void *data, size_t data_size) {
	if(tmp_buffer_count + data_size >= TEMP_BUFFER_CAPACITY) {
		fprintf(stderr, "[ERROR]: Temporary buffer capacity exceeded!");
		return NULL;
	}
	memcpy(temp_buffer + tmp_buffer_count, data, data_size);
	void *ret = temp_buffer + tmp_buffer_count;
	tmp_buffer_count += data_size;
	return ret;
}

void tmp_free(void) {
	tmp_buffer_count = 0;
}

size_t tmp_start_scratch(void) { // TODO: Error handling
	tmp_scratch_pos = tmp_buffer_count;
	return tmp_scratch_pos;
}

size_t tmp_end_scratch(void) { // TODO: Error handling
	size_t scratch_size = tmp_buffer_count - tmp_scratch_pos;
	tmp_buffer_count = tmp_scratch_pos;
	return scratch_size;
}

void *chunk_add(DigitChunk *a, DigitChunk *b, uint8_t old_carry, size_t *out_size) {
	uint8_t carry = old_carry;
	size_t scratch_offset = tmp_start_scratch();
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
	*out_size = tmp_end_scratch();
	return temp_buffer + scratch_offset;
}

DigitChunk *append_zeroed_chunk(Integer *a) {
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

Integer integer_add(Integer *a, Integer *b) {
	assert(a->head != NULL && "[ERROR]: First integer operand for '+' has no digits!");
	assert(b->head != NULL && "[ERROR]: Second integer operand for '+' has no digits!");
	Integer out = {0};
	size_t min_chunk_count = min(a->chunk_count, b->chunk_count);
	size_t max_chunk_count = max(a->chunk_count, b->chunk_count);
	DigitChunk *current_a_chunk = a->head;
	DigitChunk *current_b_chunk = b->head;
	uint8_t carry = 0;
	for(size_t i = 0; i < max_chunk_count; ++i) {
		size_t new_chunk_size = 0;
		uint8_t *digits = NULL;

		if(i < min_chunk_count) {
			digits = (uint8_t *)chunk_add(current_a_chunk, current_b_chunk, carry, &new_chunk_size);
		} else {
			DigitChunk *chunk_to_add = (current_a_chunk != NULL) ? current_a_chunk : current_b_chunk;
			digits = (uint8_t *)chunk_add(chunk_to_add, &ZeroChunk, carry, &new_chunk_size);
		}

		if(new_chunk_size > DIGIT_CHUNK_CAPACITY) {
			assert(new_chunk_size == DIGIT_CHUNK_CAPACITY + 1);
			carry = digits[new_chunk_size - 1];
		} else {
			carry = 0;
		}
		DigitChunk *appended_chunk = append_zeroed_chunk(&out);	                  
		appended_chunk->count = min(DIGIT_CHUNK_CAPACITY, new_chunk_size);        
		appended_chunk->next = NULL;                                              
		memcpy(appended_chunk->memory, digits, new_chunk_size * sizeof(*digits)); 

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

void test1(void) { // NOTE: Leaks memory...
	const char *str1 = "832987473298230000043281476942104327189472819432646328";
	const char *str2 = "335623382879320043276483204324329843243204328727671132";
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer sum = integer_add(&A, &B);
	integer_debug_print(&sum);
}

void test2(void) { // NOTE: Leaks memory...
	const char *str1 = "999999";
	const char *str2 = "1";
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer sum = integer_add(&A, &B);
	integer_debug_print(&sum);
}

int main(void) {
	test1();
	test2();
	return 0;
}
