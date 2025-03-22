#include <stdio.h>

#include <string.h>

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

// TODO: Clean up code

#define DIGIT_CHUNK_CAPACITY 5

typedef struct DigitChunk DigitChunk;
typedef struct DigitChunk {
	size_t count;
	DigitChunk *next;
	uint8_t memory[DIGIT_CHUNK_CAPACITY]; // NOTE: little endian
} DigitChunk;

DigitChunk ZeroChunk = {0};

typedef struct {
	int is_negative;
	size_t chunk_count;
	DigitChunk *head;
} Integer;

Integer integer_subtract(Integer *a, Integer *b);

Integer integer_from_str(char *str, size_t string_size) {
	// TODO: Handle -0...
	assert(string_size >= 1 && "[ERROR]: String size must be at least than 1");
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

int integer_greater(Integer *a, Integer *b) {
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
	// a and b are equal
	return 0;
}

void *chunk_add(DigitChunk *a, DigitChunk *b, uint8_t old_carry, size_t *out_size) {
	// TODO: Move tmp_start_scratch/end_scratch outside of this function
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

int chunk_subtract(DigitChunk *a, DigitChunk *b, int must_take, int *is_zero, size_t *chunk_size) {
	// TODO: Make the function signatures 'chunk_subtract' and 'chunk_add' consistent
	*is_zero = 1;
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

		if(a_digit < b_digit) {
			must_take = 1;
			// Assume we have already taken 10 from the NEXT digit...
			uint8_t diff = 10 - (b_digit - a_digit);
			if(diff != 0) {
				*is_zero = 0;
				*chunk_size += zero_count + 1;
				zero_count = 0;
			} else {
				zero_count += 1;
			}
			tmp_push(&diff, sizeof(diff));
		} else {
			must_take = 0;
			uint8_t diff = a_digit - b_digit;
			if(diff != 0) {
				*is_zero = 0;
				*chunk_size += zero_count + 1;
				zero_count = 0;
			} else {
				zero_count += 1;
			}
			tmp_push(&diff, sizeof(diff));
		}
	}
	return must_take;
}

Integer integer_subtract(Integer *a, Integer *b) {
	assert(a->head != NULL && "[ERROR]: First integer operand for '-' has no digits!");
	assert(b->head != NULL && "[ERROR]: Second integer operand for '-' has no digits!");

	Integer out = {0};
	Integer *first_sub_arg = a;
	Integer *second_sub_arg = b;
	if(!integer_greater(a, b)) { // TODO: Mark whether the integers are equal...
		first_sub_arg = b;
		second_sub_arg = a;
		out.is_negative = 1;
	}

	size_t max_chunk_count = max(a->chunk_count, b->chunk_count);
	int must_take = 0;

	DigitChunk *current_a_chunk = first_sub_arg->head;
	DigitChunk *current_b_chunk = second_sub_arg->head;
	size_t num_zero_chunks = 0;
	DigitChunk *last_chunk = NULL;
	size_t last_chunk_size;
	for(size_t i = 0; i < max_chunk_count; ++i) {
		DigitChunk *subtend_a = (current_a_chunk == NULL) ? &ZeroChunk : current_a_chunk;
		DigitChunk *subtend_b = (current_b_chunk == NULL) ? &ZeroChunk : current_b_chunk;

		size_t scratch_pos = tmp_start_scratch();
		int is_zero = 0;
		size_t chunk_size = 0;
		must_take = chunk_subtract(subtend_a, subtend_b, must_take, &is_zero, &chunk_size);
		assert(chunk_size <= DIGIT_CHUNK_CAPACITY && "[ERROR]: Chunk size after subtraction exceeds DIGIT_CHUNK_CAPACITY");
		size_t scratch_size = tmp_end_scratch();
		assert(scratch_size <= DIGIT_CHUNK_CAPACITY);

		if(is_zero) {
			num_zero_chunks += 1;
		} else {
			// append 'num_zero_chunks' ZeroDigit chunks...
			for(size_t j = 0; j < num_zero_chunks; ++j) {
				DigitChunk *zchunk = append_zeroed_chunk(&out);
				zchunk->count = DIGIT_CHUNK_CAPACITY;
			}

			// do the copy from the temp buffer into the DigitChunk...
			DigitChunk *appended_chunk = append_zeroed_chunk(&out);
			appended_chunk->count = scratch_size;
			memcpy(appended_chunk->memory, temp_buffer + scratch_pos, scratch_size);

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

	assert(must_take == 0 && "[ERROR]: Result of subtraction is negative!");
	return out;
}

int test_counter = 0;

void test1(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "832987473298230000043281476942104327189472819432646328";
	const char *str2 = "335623382879320043276483204324329843243204328727671132";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s + %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer sum = integer_add(&A, &B);
	integer_debug_print(&sum);
}

void test2(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "999999";
	const char *str2 = "1";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s + %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer sum = integer_add(&A, &B);
	integer_debug_print(&sum);
}

void test2a(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "99999";
	const char *str2 = "1";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s + %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer sum = integer_add(&A, &B);
	integer_debug_print(&sum);
}

void test3(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "1000";
	const char *str2 = "1";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test4(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "1444";
	const char *str2 = "775";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test4a(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "775";
	const char *str2 = "1444";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test5(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "99999";
	const char *str2 = "100000";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test5a(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "100011";
	const char *str2 = "100011";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test6(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str2 = "3283882";
	const char *str1 = "-998211";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s + %s = ??\n", str1, str2);
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
	test2a();
	test3();
	test4();
	test4a();
	test5();
	test5a();
	test6();
	return 0;
}
