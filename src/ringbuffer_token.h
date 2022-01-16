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
  size_t  size, mask, write_ptr, read_ptr;
} ringbuffer_token_t;

bool    ringbuffer_token_init(ringbuffer_token_t *buffer, size_t size);
bool    ringbuffer_token_put(ringbuffer_token_t *buffer, size_t *pos);
bool    ringbuffer_token_get(ringbuffer_token_t *buffer, size_t *pos);
size_t  ringbuffer_token_used(ringbuffer_token_t *buffer);
size_t  ringbuffer_token_free(ringbuffer_token_t *buffer);

#ifdef __cplusplus
}
#endif
