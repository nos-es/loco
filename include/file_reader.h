#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct ByteBuffer {
  unsigned char *data;
  size_t length;
} byte_buffer_t;

bool read_byte_buffer_from_file(const char *filepath,
                                byte_buffer_t *out_buffer);

void free_buffer(byte_buffer_t *buffer);
