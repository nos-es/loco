#pragma once
#include "info_hash.h"
#include "peer_id.h"
#include "tracker.h"

enum {
  BITTORRENT_PROTOCOL_NAME_LENGTH = 19,
  HANDSHAKE_BYTES_LENGTH = 68,
};

int connect_to_peer(const peer_t *peer);

bool handshake_with_peer(int file_descriptor, const info_hash_t *info_hash,
                         const peer_id_t *peer_id);
