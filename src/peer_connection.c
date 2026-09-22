#include "peer_connection.h"
#include "info_hash.h"
#include "peer_id.h"
#include "tracker.h"
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const unsigned char handshake_protocol[] = "BitTorrent protocol";

bool write_uint32_big_endian(unsigned char *buffer, size_t buffer_capacity,
                             uint32_t value) {

  if (buffer == NULL || buffer_capacity < 4) {
    return false;
  }

  unsigned char first_byte = value >> 24;
  unsigned char second_byte = value >> 16;
  unsigned char third_byte = value >> 8;
  unsigned char fourth_byte = value;

  buffer[0] = first_byte;
  buffer[1] = second_byte;
  buffer[2] = third_byte;
  buffer[3] = fourth_byte;

  return true;
}

int connect_to_peer(const peer_t *peer) {

  if (peer == NULL) {
    return -1;
  }

  int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

  if (file_descriptor < 0) {
    return -1;
  }

  struct sockaddr_in socket_address = {.sin_family = AF_INET,
                                       .sin_port = htons(peer->port)};

  memcpy(&socket_address.sin_addr, peer->ipv4_address, PEER_IP_LENGTH);

  int connect_result =
      connect(file_descriptor, (const struct sockaddr *)&socket_address,
              sizeof(socket_address));

  if (connect_result == -1) {
    close(file_descriptor);
    return -1;
  }

  return file_descriptor;
}

static bool build_handshake(const info_hash_t *info_hash,
                            const peer_id_t *peer_id,
                            unsigned char *out_handshake_buffer,
                            size_t handshake_capacity) {

  if (info_hash == NULL || peer_id == NULL || out_handshake_buffer == NULL) {
    return false;
  }

  if (handshake_capacity < HANDSHAKE_BYTES_LENGTH) {
    return false;
  }

  unsigned char handshake[HANDSHAKE_BYTES_LENGTH] = {0};

  // first byte contain protocol length
  handshake[0] = BITTORRENT_PROTOCOL_NAME_LENGTH;

  // write protocol name from 1 - 19
  memcpy(handshake + 1, handshake_protocol, BITTORRENT_PROTOCOL_NAME_LENGTH);

  // bytes 20 - 27 reserved

  // bytes 28 - 47 for info hash.
  size_t info_hash_offset_start = 28;
  memcpy(handshake + info_hash_offset_start, info_hash->bytes,
         INFO_HASH_LENGTH);

  // bytes 48 - 67 for peer_id
  size_t peer_id_offset_start = 48;
  memcpy(handshake + peer_id_offset_start, peer_id->bytes, PEER_ID_LENGTH);

  memcpy(out_handshake_buffer, handshake, HANDSHAKE_BYTES_LENGTH);

  return true;
}

static bool
validate_handshake_response(const unsigned char *handshake_response_buffer,
                            const info_hash_t *info_hash) {

  if (handshake_response_buffer == NULL || info_hash == NULL) {
    return false;
  }

  if (handshake_response_buffer[0] != BITTORRENT_PROTOCOL_NAME_LENGTH) {
    return false;
  }

  if (memcmp(handshake_response_buffer + 1, handshake_protocol,
             BITTORRENT_PROTOCOL_NAME_LENGTH) != 0) {
    return false;
  }

  size_t info_hash_offset_start = 28;
  if (memcmp(handshake_response_buffer + info_hash_offset_start,
             info_hash->bytes, INFO_HASH_LENGTH) != 0) {
    return false;
  }

  return true;
}

bool handshake_with_peer(int file_descriptor, const info_hash_t *info_hash,
                         const peer_id_t *peer_id) {

  unsigned char handshake_buffer[HANDSHAKE_BYTES_LENGTH] = {0};

  bool handshake_build = build_handshake(info_hash, peer_id, handshake_buffer,
                                         HANDSHAKE_BYTES_LENGTH);
  if (!handshake_build) {
    return false;
  }

  size_t send_total = 0;
  while (send_total < HANDSHAKE_BYTES_LENGTH) {

    // MSG_NOSIGNAL so that SIGPIPE can not end program.
    ssize_t sent = send(file_descriptor, handshake_buffer + send_total,
                        HANDSHAKE_BYTES_LENGTH - send_total, MSG_NOSIGNAL);

    if (sent == 0) {
      return false;
    }

    // Problem with connection? Or retry?
    if (sent == -1) {

      if (errno == EINTR) {
        continue;
      }

      return false;
    }

    send_total += sent;
  }

  unsigned char handshake_received_buffer[HANDSHAKE_BYTES_LENGTH] = {0};
  size_t received_total = 0;
  while (received_total < HANDSHAKE_BYTES_LENGTH) {

    ssize_t received =
        recv(file_descriptor, handshake_received_buffer + received_total,
             HANDSHAKE_BYTES_LENGTH - received_total, 0);

    if (received == 0) {
      return false;
    }

    // Problem with connection? Or retry?
    if (received == -1) {

      if (errno == EINTR) {
        continue;
      }

      return false;
    }

    received_total += received;
  }

  if (!validate_handshake_response(handshake_received_buffer, info_hash)) {
    return false;
  }

  return true;
}

static bool is_valid_payload_length_for_message(size_t payload_length,
                                                enum MessageId message_id) {
  switch (message_id) {

  case PEER_MESSAGE_CHOKE:
  case PEER_MESSAGE_UNCHOKE:
  case PEER_MESSAGE_INTERESTED:
  case PEER_MESSAGE_NOT_INTERESTED:
    return payload_length == 0;
  case PEER_MESSAGE_HAVE:
    return payload_length == 4;
  case PEER_MESSAGE_REQUEST:
  case PEER_MESSAGE_CANCEL:
    return payload_length == 12;
  case PEER_MESSAGE_PIECE:
    // PIECE header with min 8 bytes and max of 16 KiB Block
    return payload_length >= 8 && payload_length <= 16392;
  case PEER_MESSAGE_BITFIELD:
    // BITFIELD has variable payload lengths.
    // Exact validity is checked later with torrent/download context.
    return true;
  case PEER_MESSAGE_INVALID:
  default:
    return false;
  }
}
bool append_uint32_big_endian(unsigned char *buffer,
                              const size_t total_buffer_length,
                              size_t *byte_offset, uint32_t value) {
  if (buffer == NULL || byte_offset == NULL) {
    return false;
  }

  const size_t uint32_byte_count = 4;

  if (*byte_offset > total_buffer_length) {
    return false;
  }

  if ((total_buffer_length - *byte_offset) < uint32_byte_count) {
    return false;
  }

  bool written = write_uint32_big_endian(
      buffer + *byte_offset, total_buffer_length - *byte_offset, value);

  if (!written) {
    return false;
  }

  *byte_offset += uint32_byte_count;

  return true;
}
bool peer_send_request(int file_descriptor, uint32_t piece_index,
                       uint32_t begin, uint32_t length) {
  if (file_descriptor < 0) {
    return false;
  }

  size_t byte_offset = 0;

  uint32_t request_prefix_length_value =
      PEER_MESSAGE_REQUEST_LENGTH - LENGTH_PREFIX_SIZE;

  unsigned char request_buffer[PEER_MESSAGE_REQUEST_LENGTH];

  // write prefix length to buffer
  bool prefix_len_appended =
      append_uint32_big_endian(request_buffer, PEER_MESSAGE_REQUEST_LENGTH,
                               &byte_offset, request_prefix_length_value);
  if (!prefix_len_appended) {
    return false;
  }

  // write message_id to buffer
  request_buffer[byte_offset] = PEER_MESSAGE_REQUEST;
  byte_offset += 1;

  // write piece index to buffer
  bool piece_index_appended = append_uint32_big_endian(
      request_buffer, PEER_MESSAGE_REQUEST_LENGTH, &byte_offset, piece_index);
  if (!piece_index_appended) {
    return false;
  }

  // write begin to buffer.
  bool begin_appended = append_uint32_big_endian(
      request_buffer, PEER_MESSAGE_REQUEST_LENGTH, &byte_offset, begin);

  if (!begin_appended) {
    return false;
  }

  // length begin to buffer.
  bool length_appended = append_uint32_big_endian(
      request_buffer, PEER_MESSAGE_REQUEST_LENGTH, &byte_offset, length);

  if (!length_appended) {
    return false;
  }

  if (byte_offset != PEER_MESSAGE_REQUEST_LENGTH) {
    return false;
  }

  size_t send_total = 0;
  while (send_total < PEER_MESSAGE_REQUEST_LENGTH) {

    ssize_t sent = send(file_descriptor, request_buffer + send_total,
                        PEER_MESSAGE_REQUEST_LENGTH - send_total, MSG_NOSIGNAL);

    if (sent == 0) {
      return false;
    }

    // Problem with connection? Or retry?
    if (sent == -1) {

      // systemcall was interrupted by a signal.
      if (errno == EINTR) {
        continue;
      }

      return false;
    }

    send_total += sent;
  }
  return true;
}

bool peer_send_interested(int file_descriptor) {

  if (file_descriptor < 0) {
    return false;
  }

  uint32_t interested_prefix_length = 1;
  unsigned char interested_msg_buffer[PEER_MESSAGE_INTERESTED_LENGTH];

  bool uint32_big_endian_written = write_uint32_big_endian(
      interested_msg_buffer, PEER_MESSAGE_INTERESTED_LENGTH,
      interested_prefix_length);

  if (!uint32_big_endian_written) {
    return false;
  }

  interested_msg_buffer[PEER_MESSAGE_INTERESTED_LENGTH - 1] =
      PEER_MESSAGE_INTERESTED;

  size_t msg_buffer_length = sizeof(interested_msg_buffer);

  size_t send_total = 0;
  while (send_total < msg_buffer_length) {

    ssize_t sent = send(file_descriptor, interested_msg_buffer + send_total,
                        msg_buffer_length - send_total, MSG_NOSIGNAL);

    if (sent == 0) {
      return false;
    }

    // Problem with connection? Or retry?
    if (sent == -1) {

      // systemcall was interrupted by a signal.
      if (errno == EINTR) {
        continue;
      }

      return false;
    }

    send_total += sent;
  }

  return true;
}

bool receive_peer_wire_message(int file_descriptor,
                               peer_wire_message_t *out_message) {

  if (out_message == NULL || file_descriptor < 0) {
    return false;
  }

  if (out_message->message_id != PEER_MESSAGE_INVALID ||
      out_message->is_keep_alive == true || out_message->payload != NULL ||
      out_message->payload_length != 0) {
    return false;
  }

  size_t received_total = 0;
  size_t prefix_byte_length = 4;
  unsigned char prefix_length_buffer[4] = {0};

  while (received_total < prefix_byte_length) {
    printf("Waiting for peer message...\n");
    ssize_t received =
        recv(file_descriptor, prefix_length_buffer + received_total,
             prefix_byte_length - received_total, 0);

    printf("received value: %zd\n", received);
    if (received == 0) {
      printf("failed reading length prefix (received 0)\n");
      return false;
    }

    if (received == -1) {

      if (errno == EINTR) {
        continue;
      }

      printf("failed reading length prefix (errno)\n");
      return false;
    }

    received_total += received;
  }
  uint32_t first_byte = (uint32_t)prefix_length_buffer[0];
  uint32_t second_byte = (uint32_t)prefix_length_buffer[1];
  uint32_t third_byte = (uint32_t)prefix_length_buffer[2];
  uint32_t fourth_byte = (uint32_t)prefix_length_buffer[3];

  uint32_t prefix_length = (first_byte << 24) | (second_byte << 16) |
                           (third_byte << 8) | fourth_byte;

  printf("Prefix Length: %" PRIu32 "\n", prefix_length);

  // Keep-Alive-Message
  if (prefix_length == 0) {
    out_message->message_id = PEER_MESSAGE_INVALID;
    out_message->is_keep_alive = true;
    out_message->payload = NULL;
    out_message->payload_length = 0;
    return true;
  }

  // Message ID Byte
  received_total = 0;

  const size_t message_id_length = 1;
  unsigned char message_id_buffer[1] = {0};

  while (received_total < message_id_length) {

    ssize_t received =
        recv(file_descriptor, message_id_buffer, message_id_length, 0);

    if (received == 0) {
      printf("Failed reading message id\n");
      return false;
    }

    if (received == -1) {

      if (errno == EINTR) {
        continue;
      }

      printf("Failed reading message id\n");
      return false;
    }

    received_total += received;
  }

  if (message_id_buffer[0] >= PEER_MESSAGE_INVALID) {
    printf("Unsupported message id: %u\n", (unsigned int)message_id_buffer[0]);
    // Unsupported message id.
    return false;
  }

  printf("message id: %u\n", (unsigned int)message_id_buffer[0]);

  size_t payload_length = prefix_length - message_id_length;
  enum MessageId message_id = (enum MessageId)message_id_buffer[0];

  if (payload_length > MAX_PAYLOAD_LENGTH ||
      !is_valid_payload_length_for_message(payload_length, message_id)) {
    printf("Invalid payload length\n");
    return false;
  }

  // Payload
  if (payload_length == 0) {
    out_message->message_id = message_id;
    out_message->is_keep_alive = false;
    out_message->payload = NULL;
    out_message->payload_length = 0;
    return true;
  }

  received_total = 0;

  unsigned char *payload_buffer = malloc(payload_length);

  if (payload_buffer == NULL) {
    printf("Failed allocating payload\n");
    return false;
  }

  while (received_total < payload_length) {

    ssize_t received = recv(file_descriptor, payload_buffer + received_total,
                            payload_length - received_total, 0);

    if (received == 0) {
      printf("Failed reading payload\n");
      free(payload_buffer);
      return false;
    }

    if (received == -1) {

      if (errno == EINTR) {
        continue;
      }

      printf("Failed reading payload\n");
      free(payload_buffer);
      return false;
    }

    received_total += received;
  }

  out_message->message_id = message_id;
  out_message->is_keep_alive = false;
  out_message->payload = payload_buffer;
  out_message->payload_length = payload_length;

  return true;
}

bool determine_piece_info_from_piece_payload(peer_wire_message_t *piece_message,
                                             size_t *out_piece_index,
                                             size_t *out_begin,
                                             const unsigned char **out_block,
                                             size_t *out_block_length) {

  if (piece_message == NULL || out_piece_index == NULL || out_begin == NULL ||
      out_block == NULL || out_block_length == NULL) {
    return false;
  }

  if (piece_message->message_id != PEER_MESSAGE_PIECE ||
      piece_message->payload_length < 8) {
    return false;
  }

  if (piece_message->payload == NULL) {
    return false;
  }

  uint32_t piece_index = ((uint32_t)piece_message->payload[0] << 24) |
                         ((uint32_t)piece_message->payload[1] << 16) |
                         ((uint32_t)piece_message->payload[2] << 8) |
                         ((uint32_t)piece_message->payload[3]);

  uint32_t begin = ((uint32_t)piece_message->payload[4] << 24) |
                   ((uint32_t)piece_message->payload[5] << 16) |
                   ((uint32_t)piece_message->payload[6] << 8) |
                   ((uint32_t)piece_message->payload[7]);

  *out_piece_index = (size_t)piece_index;
  *out_begin = (size_t)begin;
  *out_block = piece_message->payload + 8;
  *out_block_length = piece_message->payload_length - 8;

  return true;
}
bool update_bitfield(peer_connection_t *peer_connection,
                     const unsigned char *received_have_payload,
                     const size_t payload_length, const size_t piece_count) {

  if (peer_connection == NULL || received_have_payload == NULL ||
      payload_length != PEER_MESSAGE_HAVE_PAYLOAD_LENGTH) {
    return false;
  }

  unsigned char piece_index_buffer[PEER_MESSAGE_HAVE_PAYLOAD_LENGTH];
  memcpy(piece_index_buffer, received_have_payload,
         PEER_MESSAGE_HAVE_PAYLOAD_LENGTH);

  uint32_t piece_index_wire = ((uint32_t)piece_index_buffer[0] << 24) |
                              ((uint32_t)piece_index_buffer[1] << 16) |
                              ((uint32_t)piece_index_buffer[2] << 8) |
                              ((uint32_t)piece_index_buffer[3]);

  size_t piece_index = (size_t)piece_index_wire;
  printf("Peer sent HAVE message for Piece %zu\n", piece_index);

  if (piece_index >= piece_count) {
    return false;
  }

  size_t bitfield_length = (piece_count + 7) / 8;

  if (peer_connection->peer_piece_bitfield.bytes != NULL &&
      peer_connection->peer_piece_bitfield.length != bitfield_length) {
    return false;
  }
  if (peer_connection->peer_piece_bitfield.bytes == NULL &&
      peer_connection->peer_piece_bitfield.length != 0) {
    return false;
  }

  // Initialize bitfield if not existing.
  if (peer_connection->peer_piece_bitfield.bytes == NULL) {

    unsigned char *temp_bitfield =
        calloc(bitfield_length, sizeof(unsigned char));

    if (temp_bitfield == NULL) {
      return false;
    }

    peer_connection->peer_piece_bitfield.bytes = temp_bitfield;
    peer_connection->peer_piece_bitfield.length = bitfield_length;
  }

  size_t byte_index = piece_index / 8;
  size_t bit_position = piece_index % 8;
  unsigned char mask = 0x80 >> bit_position;
  peer_connection->peer_piece_bitfield.bytes[byte_index] |= mask;

  return true;
}

bool bitfield_applied(peer_connection_t *peer_connection,
                      const unsigned char *received_bitfield_payload,
                      const size_t payload_length, const size_t piece_count) {

  if (peer_connection == NULL || received_bitfield_payload == NULL) {
    return false;
  }
  if (peer_connection->peer_piece_bitfield.bytes != NULL ||
      peer_connection->peer_piece_bitfield.length != 0) {
    return false;
  }

  size_t expected_length = (piece_count + 7) / 8;

  if (payload_length != expected_length) {
    return false;
  }

  size_t remainder = piece_count % 8;
  if (remainder != 0) {
    size_t padding_bits = 8 - remainder;
    unsigned char padding_mask = (1 << padding_bits) - 1;

    unsigned char last_bitfield_byte =
        received_bitfield_payload[payload_length - 1];

    if ((padding_mask & last_bitfield_byte) != 0) {
      return false;
    }
  }

  unsigned char *temp_bitfield = malloc(payload_length);
  if (temp_bitfield == NULL) {
    return false;
  }

  memcpy(temp_bitfield, received_bitfield_payload, payload_length);

  peer_connection->peer_piece_bitfield.bytes = temp_bitfield;

  peer_connection->peer_piece_bitfield.length = payload_length;

  return true;
}

bool process_incoming_piece_message(piece_download_state_t *current_piece_state,
                                    peer_wire_message_t *msg) {

  if (current_piece_state == NULL || msg == NULL ||
      current_piece_state->piece_buffer == NULL) {
    return false;
  }

  if (!current_piece_state->request_pending) {
    return false;
  }

  size_t piece_index = 0;
  size_t begin = 0;
  size_t block_length = 0;
  const unsigned char *block_buffer = NULL;

  bool piece_info_determined = determine_piece_info_from_piece_payload(
      msg, &piece_index, &begin, &block_buffer, &block_length);

  if (!piece_info_determined) {
    return false;
  }

  // validate piece info
  if (piece_index != current_piece_state->piece_index ||
      begin != current_piece_state->requested_begin ||
      block_length != current_piece_state->requested_length) {
    return false;
  }

  // check overflow.
  if (begin > current_piece_state->piece_size ||
      block_length > current_piece_state->piece_size - begin) {
    return false;
  }

  memcpy(current_piece_state->piece_buffer + begin, block_buffer, block_length);

  current_piece_state->bytes_received += block_length;
  current_piece_state->request_pending = false;
  return true;
}

void free_peer_wire_message(peer_wire_message_t *message) {
  message->message_id = PEER_MESSAGE_INVALID;
  message->is_keep_alive = false;
  free(message->payload);
  message->payload = NULL;
  message->payload_length = 0;
}
