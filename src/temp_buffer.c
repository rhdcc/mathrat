#include "temp_buffer.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

uint8_t temp_buffer[TEMP_BUFFER_CAPACITY];
size_t tmp_buffer_count = 0;
size_t tmp_scratch_pos = 0;
bool tmp_modifying_scratch = false;

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

void *tmp_start_scratch(void) {
    assert(tmp_modifying_scratch == false && "[ERROR]: Cannot begin a scratch session without ending the previous session first.");
    tmp_scratch_pos = tmp_buffer_count;
    tmp_modifying_scratch = true;
    return (void *)(temp_buffer + tmp_scratch_pos);
}

size_t tmp_end_scratch(void) {
    assert(tmp_modifying_scratch == true && "[ERROR]: Cannot end a scratch session without starting a previous session first.");
    size_t scratch_size = tmp_buffer_count - tmp_scratch_pos;
    tmp_buffer_count = tmp_scratch_pos;
    tmp_modifying_scratch = false;
    return scratch_size;
}
