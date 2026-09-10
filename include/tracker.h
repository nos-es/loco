#pragma once
#include "bencode_types.h"
#include "info_hash.h"
#include "peer_id.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
  PEER_IP_LENGTH = 4,
  PEER_SEGMENT_LENGTH = 6,
};
typedef struct Peer {
  uint8_t ipv4_address[PEER_IP_LENGTH];
  uint16_t port;
} peer_t;

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

bool parse_peer_from_segment(const bencode_segment_t *peer_segment,
                             peer_t *out_peer);

bool tracker_response_parse(const tracker_response_buffer_t *response,
                            bencode_object_t *out_parsed_obj);

bool find_interval(const bencode_object_t *response_obj, int64_t *out_interval);

bool find_peers(const bencode_object_t *response_obj,
                bencode_segment_t *out_peers);

bool peers_extract(const bencode_segment_t *peers_segment, peer_t **out_peers,
                   size_t *out_peer_count);
