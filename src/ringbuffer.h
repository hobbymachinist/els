#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t *data;
  size_t  size, mask, write_ptr, read_ptr;
} ringbuffer_t;

bool ringbuffer_init(ringbuffer_t *buffer, size_t size, void *data);

bool ringbuffer_putc(ringbuffer_t *buffer, uint8_t byte);
bool ringbuffer_getc(ringbuffer_t *buffer, uint8_t *byte);
bool ringbuffer_peek(ringbuffer_t *buffer, uint8_t *byte);

size_t ringbuffer_used(ringbuffer_t *buffer);
size_t ringbuffer_free(ringbuffer_t *buffer);

#ifdef __cplusplus
}
#endif
