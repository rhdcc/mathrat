#ifndef TEMP_BUFFER_H
#define TEMP_BUFFER_H

#include <stdint.h> // uint8_t
#include <stddef.h> // size_t

#define TEMP_BUFFER_CAPACITY 1024
extern uint8_t temp_buffer[TEMP_BUFFER_CAPACITY];

void *tmp_push(void *data, size_t data_size);
void tmp_free(void);
void *tmp_start_scratch(void);
size_t tmp_end_scratch(void);

#endif // TEMP_BUFFER_H
