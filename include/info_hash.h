#pragma once
#include <stdbool.h>
#include <stddef.h>

enum { INFO_HASH_LENGTH = 20 };

typedef struct InfoHash {
  unsigned char bytes[INFO_HASH_LENGTH];
} info_hash_t;

bool compute_info_hash(const unsigned char *bytes, size_t length,
                        info_hash_t *out_info_hash);
