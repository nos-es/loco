#pragma once
#include "info_hash.h"
#include <stdbool.h>
#include <stdint.h>

enum { PEER_ID_LENGTH = 20 };

typedef struct TrackerRequest {
  info_hash_t info_hash;
  unsigned char peer_id[PEER_ID_LENGTH];
  uint64_t port;
  uint64_t uploaded;
  uint64_t downloaded;
  uint64_t left;
  bool compact;
} tracker_request_t;
