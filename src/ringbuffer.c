#include "ringbuffer.h"

bool ringbuffer_init(ringbuffer_t *buffer, size_t size, void *data) {
  // not a power of 2.
  if ((size & (size -1)) != 0)
    return false;

  memset(buffer, 0, sizeof(ringbuffer_t));
  buffer->data = data;
  buffer->size = size;
  buffer->mask = size - 1;
  return true;
}

bool ringbuffer_putc(ringbuffer_t *buffer, uint8_t byte) {
  if (((buffer->write_ptr - buffer->read_ptr) & buffer->mask) == buffer->mask)
    return false;

  buffer->data[buffer->write_ptr] = byte;
  buffer->write_ptr = (buffer->write_ptr + 1) & buffer->mask;
  return true;
}

bool ringbuffer_getc(ringbuffer_t *buffer, uint8_t *byte) {
  if (((buffer->write_ptr - buffer->read_ptr) & buffer->mask) <= 0)
    return false;

  *byte = buffer->data[buffer->read_ptr];
  buffer->read_ptr = (buffer->read_ptr + 1) & buffer->mask;
  return true;
}

bool ringbuffer_peek(ringbuffer_t *buffer, uint8_t *byte) {
  if (((buffer->write_ptr - buffer->read_ptr) & buffer->mask) <= 0)
    return false;
  *byte = buffer->data[buffer->read_ptr];
  return true;
}

size_t ringbuffer_used(ringbuffer_t *buffer) {
  return (buffer->write_ptr - buffer->read_ptr) & buffer->mask;
}

size_t ringbuffer_free(ringbuffer_t *buffer) {
  return buffer->size - ((buffer->write_ptr - buffer->read_ptr) & buffer->mask);
}
