#pragma once
#include "bencode_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct BencodedSpan {
  size_t start_offset;
  size_t length;
} bencoded_span_t;

typedef struct TorrentInfo {
  bencode_segment_t name;
  int64_t length;
  int64_t piece_length;
  bencode_segment_t pieces;
  bencoded_span_t info_span;
} torrent_info_t;

bool torrent_metadata_extract_info(const bencode_object_t *root,
                                   torrent_info_t *out_info);
const bencode_object_t *
torrent_metadata_find_info(const bencode_object_t *root);

const bencode_object_t *
torrent_metadata_find_name(const bencode_object_t *info);

const bencode_object_t *
torrent_metadata_find_length(const bencode_object_t *info);

const bencode_object_t *
torrent_metadata_find_piece_length(const bencode_object_t *info);

const bencode_object_t *
torrent_metadata_find_pieces(const bencode_object_t *info);
