#pragma once
#include "bencode_types.h"
#include "info_hash.h"
#include "peer_id.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum tracker_event {
  TRACKER_EVENT_NONE,
  TRACKER_EVENT_STARTED,
  TRACKER_EVENT_COMPLETED,
  TRACKER_EVENT_STOPPED
};

typedef struct TrackerRequest {
  info_hash_t info_hash;
  peer_id_t peer_id;
  uint16_t port;
  uint64_t uploaded;
  uint64_t downloaded;
  uint64_t left;
  bool compact;
  enum tracker_event event;
} tracker_request_t;

typedef struct TrackerResponseBuffer {
  unsigned char *data;
  size_t length;
} tracker_response_buffer_t;

char *build_tracker_url(const bencode_segment_t *announce,
                        const tracker_request_t *request);

size_t write_chunk_to_tracker_response_buffer(char *chunk, size_t size,
                                              size_t nmemb,
                                              void *tracker_response_buffer);

bool tracker_announce(const bencode_segment_t *announce,
                      const tracker_request_t *request,
                      tracker_response_buffer_t *out_response);

bool tracker_response_parse(const tracker_response_buffer_t *response,
                            bencode_object_t *out_parsed_obj);
