#include "peer_connection.h"
#include "info_hash.h"
#include "peer_id.h"
#include "tracker.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const unsigned char handshake_protocol[] = "BitTorrent protocol";
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

bool receive_peer_wire_message(int file_descriptor,
                               peer_wire_message_t *out_message) {

  if (out_message == NULL || file_descriptor < 0) {
    return false;
  }
  size_t received_total = 0;
  size_t prefix_byte_length = 4;
  unsigned char prefix_length_buffer[4] = {0};
  while (received_total < prefix_byte_length) {

    ssize_t received =
        recv(file_descriptor, prefix_length_buffer + received_total,
             prefix_byte_length - received_total, 0);

    if (received == 0) {
      return false;
    }

    if (received == -1) {

      if (errno == EINTR) {
        continue;
      }

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

  return false;
}
