#include "ringbuffer_token.h"

bool ringbuffer_token_init(ringbuffer_token_t *buffer, size_t size) {
  // not a power of 2.
  if ((size & (size -1)) != 0)
    return false;

  memset(buffer, 0, sizeof(ringbuffer_token_t));
  buffer->size = size;
  buffer->mask = size - 1;
  return true;
}

bool ringbuffer_token_put(ringbuffer_token_t *buffer, size_t *pos) {
  if (((buffer->write_ptr - buffer->read_ptr) & buffer->mask) == buffer->mask)
    return false;

  *pos = buffer->write_ptr;
  buffer->write_ptr = (buffer->write_ptr + 1) & buffer->mask;
  return true;
}

bool ringbuffer_token_get(ringbuffer_token_t *buffer, size_t *pos) {
  if (((buffer->write_ptr - buffer->read_ptr) & buffer->mask) <= 0)
    return false;

  *pos = buffer->read_ptr;
  buffer->read_ptr = (buffer->read_ptr + 1) & buffer->mask;
  return true;
}

size_t ringbuffer_token_used(ringbuffer_token_t *buffer) {
  return (buffer->write_ptr - buffer->read_ptr) & buffer->mask;
}

size_t ringbuffer_token_free(ringbuffer_token_t *buffer) {
  return buffer->size - ((buffer->write_ptr - buffer->read_ptr) & buffer->mask);
}
