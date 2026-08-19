#pragma once
#include "bencode_types.h"

const bencode_object_t *
torrent_metadata_find_info(const bencode_object_t *root);

const bencode_object_t *
torrent_metadata_find_name(const bencode_object_t *info);
