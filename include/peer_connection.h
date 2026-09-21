#pragma once
#include "info_hash.h"
#include "peer_id.h"
#include "tracker.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
  BITTORRENT_PROTOCOL_NAME_LENGTH = 19,
  HANDSHAKE_BYTES_LENGTH = 68,
  PEER_MESSAGE_INTERESTED_LENGTH = 5,
  PEER_MESSAGE_REQUEST_LENGTH = 17,
  LENGTH_PREFIX_SIZE = 4,
  DEFAULT_REQUEST_BLOCK_SIZE = 16384,
  MAX_PAYLOAD_LENGTH = 1048576,
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

typedef struct PieceBitfield {
  unsigned char *bytes;
  size_t length;
} piece_bitfield_t;

typedef struct PeerConnection {
  piece_bitfield_t peer_piece_bitfield;
  peer_t peer;
  bool peer_choking_us;
  bool we_are_interested;
  int socket;
} peer_connection_t;

typedef struct PeerWireMessage {
  enum MessageId message_id;
  unsigned char *payload;
  size_t payload_length;
  bool is_keep_alive;
} peer_wire_message_t;

typedef struct PieceDownloadState {
  size_t piece_index;
  size_t piece_size;
  unsigned char *piece_buffer;
  size_t bytes_received;
  bool request_pending;
  uint32_t requested_begin;
  uint32_t requested_length;
} piece_download_state_t;

int connect_to_peer(const peer_t *peer);

bool handshake_with_peer(int file_descriptor, const info_hash_t *info_hash,
                         const peer_id_t *peer_id);

bool receive_peer_wire_message(int file_descriptor,
                               peer_wire_message_t *out_message);

bool bitfield_applied(peer_connection_t *peer_connection,
                      const unsigned char *received_bitfield_payload,
                      const size_t payload_length, const size_t piece_count);

bool peer_send_interested(int file_descriptor);

bool peer_send_request(int file_descriptor, uint32_t piece_index,
                       uint32_t begin, uint32_t length);

bool process_incoming_piece_message(piece_download_state_t *current_piece_state,
                                    peer_wire_message_t *msg);

bool determine_piece_info_from_piece_payload(peer_wire_message_t *piece_message,
                                             size_t *out_piece_index,
                                             size_t *out_begin,
                                             const unsigned char **out_block,
                                             size_t *out_block_length);

void free_peer_wire_message(peer_wire_message_t *message);
