#pragma once
#include "info_hash.h"
#include "peer_id.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct TrackerRequest {
  info_hash_t info_hash;
  peer_id_t peer_id;
  uint16_t port;
  uint64_t uploaded;
  uint64_t downloaded;
  uint64_t left;
  bool compact;
} tracker_request_t;
