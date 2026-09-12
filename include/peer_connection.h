#pragma once
#include "info_hash.h"
#include "peer_id.h"
#include "tracker.h"
#include <stdbool.h>
#include <stddef.h>

enum {
  BITTORRENT_PROTOCOL_NAME_LENGTH = 19,
  HANDSHAKE_BYTES_LENGTH = 68,
};

enum MessageId {
  PEER_MESSAGE_CHOKE = 0,
  PEER_MESSAGE_UNCHOKE = 1,
  PEER_MESSAGE_INTERESTED = 2,
  PEER_MESSAGE_NOT_INTERESTED = 3,
  PEER_MESSAGE_HAVE = 4,
  PEER_MESSAGE_BITFIELD = 5,
  PEER_MESSAGE_REQUEST = 6,
  PEER_MESSAGE_PIECE = 7,
  PEER_MESSAGE_CANCEL = 8,
  PEER_MESSAGE_INVALID = 9
};

typedef struct PeerWireMessage {
  enum MessageId message_id;
  unsigned char *payload;
  size_t payload_length;
  bool is_keep_alive;
} peer_wire_message_t;

int connect_to_peer(const peer_t *peer);

bool handshake_with_peer(int file_descriptor, const info_hash_t *info_hash,
                         const peer_id_t *peer_id);

bool receive_peer_wire_message(int file_descriptor,
                               peer_wire_message_t *out_message);
