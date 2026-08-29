#pragma once
#include "bencode_types.h"
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

char *build_tracker_url(const bencode_segment_t *announce,
                        const tracker_request_t *request);
