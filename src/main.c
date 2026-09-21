#include "bencode_parser.h"
#include "bencode_types.h"
#include "cli.h"
#include "file_reader.h"
#include "info_hash.h"
#include "peer_connection.h"
#include "peer_id.h"
#include "torrent_metadata.h"
#include "tracker.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[]) {

  const char *torrent_filepath = NULL;

  if (!cli_arguments_valid(argc, argv, &torrent_filepath)) {
    fprintf(stderr, "Usage: %s <torrent-file>\n", argv[0]);
    return 1;
  }

  byte_buffer_t buffer = {.data = NULL, .length = 0};

  bool result = read_byte_buffer_from_file(torrent_filepath, &buffer);
  if (!result) {
    fprintf(stderr, "Failed to read file.\n");
    free_buffer(&buffer);
    return 1;
  }

  parser_state_t parser;
  bool parse_result = bencode_parser_init(&parser, buffer.data, buffer.length);

  if (!parse_result) {
    free_buffer(&buffer);
    return 1;
  }

  bencode_object_t obj = {.type = INVALID, .value.integer = 9870};

  bool is_parsed = parse_bencode_buffer(&parser, &obj);

  if (!is_parsed) {
    fprintf(stderr, "Failed to parse file.\n");
    free_buffer(&buffer);
    return 1;
  }

  torrent_metadata_t torrent_metadata = {0};

  bool metadata_extracted = torrent_metadata_extract(&obj, &torrent_metadata);

  if (!metadata_extracted) {
    fprintf(stderr, "Failed to extract torrent metadata.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }
  info_hash_t info_hash = {.bytes = {0}};
  torrent_info_t *torrent_info = &torrent_metadata.info;

  if (torrent_info->info_span.start_offset > buffer.length) {

    fprintf(stderr, "Info start offset greater than buffer length.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  if (torrent_info->info_span.length >
      buffer.length - torrent_info->info_span.start_offset) {
    fprintf(stderr, "buffer length exceeded.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  bool computed =
      compute_info_hash(&buffer.data[torrent_info->info_span.start_offset],
                        torrent_info->info_span.length, &info_hash);
  if (!computed) {
    fprintf(stderr, "Failed to compute info hash.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  peer_id_t peer_id = {0};

  bool peer_id_generated = generate_peer_id(&peer_id);
  if (!peer_id_generated) {
    fprintf(stderr, "Failed to generate peer id.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  tracker_request_t tracker_request = {.info_hash = info_hash,
                                       .peer_id = peer_id,
                                       .port = 0,
                                       .uploaded = 0,
                                       .downloaded = 0,
                                       .left = torrent_metadata.info.length,
                                       .compact = true,
                                       .event = TRACKER_EVENT_STARTED};

  tracker_response_buffer_t tracker_response = {0};
  bool announced = tracker_announce(&torrent_metadata.announce,
                                    &tracker_request, &tracker_response);
  if (!announced) {
    fprintf(stderr, "Failed to announce to tracker.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  printf("Tracker response length: %zu bytes\n", tracker_response.length);

  bencode_object_t response_obj = {.type = INVALID};

  bool response_parsed =
      tracker_response_parse(&tracker_response, &response_obj);

  if (!response_parsed) {
    fprintf(stderr, "Failed to parse tracker response.\n");
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  int64_t interval = 0;
  bool interval_found = find_interval(&response_obj, &interval);

  if (!interval_found) {
    fprintf(stderr, "Interval not found.\n");
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }
  printf("Interval: "
         "%" PRId64 "\n",
         interval);

  bencode_segment_t peers_segment = {0};
  bool peers_found = find_peers(&response_obj, &peers_segment);

  if (!peers_found) {
    fprintf(stderr, "Peers not found.\n");
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  printf("Peers length: %zu bytes\n", peers_segment.length);

  peer_t *current_peers = NULL;
  size_t peer_count = 0;

  bool peers_extracted =
      peers_extract(&peers_segment, &current_peers, &peer_count);

  if (!peers_extracted) {
    fprintf(stderr, "Peers could not be extracted.\n");
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  printf("Peer count: %zu\n", peer_count);
  for (size_t i = 0; i < peer_count; i++) {
    printf("\n");
    printf("Peer: %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 ":%" PRIu16 "",
           current_peers[i].ipv4_address[0], current_peers[i].ipv4_address[1],
           current_peers[i].ipv4_address[2], current_peers[i].ipv4_address[3],
           current_peers[i].port);
    printf("\n");
  }

  if (peer_count == 0) {
    fprintf(stderr, "No Peers available.\n");
    free(current_peers);
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  int fd = connect_to_peer(&current_peers[0]);

  if (fd < 0) {
    fprintf(stderr, "Peer connection failed.\n");
    free(current_peers);
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  printf("Connected to Peer: %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8
         ":%" PRIu16 "",
         current_peers[0].ipv4_address[0], current_peers[0].ipv4_address[1],
         current_peers[0].ipv4_address[2], current_peers[0].ipv4_address[3],
         current_peers[0].port);
  printf("\n");

  bool handshake_success = handshake_with_peer(fd, &info_hash, &peer_id);

  if (!handshake_success) {
    fprintf(stderr, "Handshake failed.\n");
    close(fd);
    free(current_peers);
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }
  printf("Handshake with Peer: %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8
         ":%" PRIu16 " successful!\n",
         current_peers[0].ipv4_address[0], current_peers[0].ipv4_address[1],
         current_peers[0].ipv4_address[2], current_peers[0].ipv4_address[3],
         current_peers[0].port);
  printf("\n");

  if (torrent_info->piece_length <= 0 || torrent_info->length < 0) {
    close(fd);
    free(current_peers);
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  if ((uintmax_t)torrent_info->piece_length > (uintmax_t)SIZE_MAX ||
      (uintmax_t)torrent_info->length > (uintmax_t)SIZE_MAX) {
    close(fd);
    free(current_peers);
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  size_t piece_count = torrent_info->pieces.length / 20;

  if (piece_count == 0) {
    fprintf(stderr, "Piece count is 0.\n");
    close(fd);
    free(current_peers);
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  bool connection_active = true;
  size_t total_file_size = (size_t)torrent_info->length;
  size_t normal_piece_size = (size_t)torrent_info->piece_length;
  peer_wire_message_t current_peer_wire_message = {.message_id =
                                                       PEER_MESSAGE_INVALID};

  peer_connection_t current_connection = {
      .peer_piece_bitfield = {.bytes = NULL, .length = 0},
      .peer = current_peers[0],
      .peer_choking_us = true,
      .we_are_interested = false,
      .socket = fd};

  size_t first_piece_size = normal_piece_size;

  if (piece_count == 1) {
    first_piece_size = total_file_size;
  }

  unsigned char *piece_buffer = malloc(first_piece_size);

  if (piece_buffer == NULL) {
    fprintf(stderr, "Buffer memory for pieces could not be allocated.\n");
    close(fd);
    free(current_peers);
    free_bencode_object(&response_obj);
    free(tracker_response.data);
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  piece_download_state_t current_piece_state = {.piece_index = 0,
                                                .piece_size = first_piece_size,
                                                .piece_buffer = piece_buffer};

  while (connection_active) {

    free_peer_wire_message(&current_peer_wire_message);

    bool received_msg =
        receive_peer_wire_message(fd, &current_peer_wire_message);

    if (!received_msg) {
      fprintf(stderr, "No valid peer wire message received.\n");

      connection_active = false;
      break;
    }

    if (current_peer_wire_message.is_keep_alive) {
      continue;
    }

    switch (current_peer_wire_message.message_id) {

    case PEER_MESSAGE_CHOKE:
      current_connection.peer_choking_us = true;
      break;
    case PEER_MESSAGE_UNCHOKE:
      current_connection.peer_choking_us = false;
      break;
    case PEER_MESSAGE_INTERESTED:
      break;
    case PEER_MESSAGE_NOT_INTERESTED:
      break;
    case PEER_MESSAGE_HAVE:
      break;
    case PEER_MESSAGE_BITFIELD: {

      bool bitfield_payload_applied = bitfield_applied(
          &current_connection, current_peer_wire_message.payload,
          current_peer_wire_message.payload_length, piece_count);

      if (!bitfield_payload_applied) {

        connection_active = false;
        continue;
      }
      break;
    }
    case PEER_MESSAGE_REQUEST:
      break;
    case PEER_MESSAGE_PIECE: {

      if (!process_incoming_piece_message(&current_piece_state,
                                          &current_peer_wire_message)) {
        connection_active = false;
        continue;
      }

      break;
    }
    case PEER_MESSAGE_CANCEL:
      break;
    case PEER_MESSAGE_INVALID:
      break;
    }

    // bitfield exists from this peer.
    if (current_connection.peer_piece_bitfield.bytes != NULL) {

      if (current_piece_state.piece_index >= piece_count) {
        break;
      }

      size_t byte_index = current_piece_state.piece_index / 8;
      size_t bit_position = current_piece_state.piece_index % 8;
      unsigned char mask = 0x80 >> bit_position;

      if (byte_index >= current_connection.peer_piece_bitfield.length) {
        break;
      }

      // Peer does not have current piece.
      if ((current_connection.peer_piece_bitfield.bytes[byte_index] & mask) ==
          0) {
        break;
      }

      if (current_piece_state.bytes_received > current_piece_state.piece_size) {
        break;
      }

      // all bytes for this piece received
      if (current_piece_state.bytes_received ==
          current_piece_state.piece_size) {
        // TODO: SHA1 Check
        break;
      }

      if (current_connection.we_are_interested == false) {
        bool interested_sent = peer_send_interested(fd);
        if (!interested_sent) {
          break;
        }
        current_connection.we_are_interested = true;
      }

      // When true send request.
      if (current_connection.we_are_interested == true &&
          current_connection.peer_choking_us == false &&
          current_piece_state.request_pending == false) {

        size_t remaining =
            current_piece_state.piece_size - current_piece_state.bytes_received;
        size_t request_begin = current_piece_state.bytes_received;
        size_t request_length = DEFAULT_REQUEST_BLOCK_SIZE;

        if (remaining < DEFAULT_REQUEST_BLOCK_SIZE) {
          request_length = remaining;
        }

        // check uint32_t overflow before sending request.
        if (current_piece_state.piece_index > UINT32_MAX ||
            current_piece_state.bytes_received > UINT32_MAX) {
          break;
        }

        bool requested = peer_send_request(
            fd, (uint32_t)current_piece_state.piece_index,
            (uint32_t)request_begin, (uint32_t)request_length);

        if (!requested) {
          break;
        }

        current_piece_state.request_pending = true;
        current_piece_state.requested_length = request_length;
        current_piece_state.requested_begin = request_begin;
      }
    }
  }

  // Cleanup connection.
  free(current_connection.peer_piece_bitfield.bytes);
  free_peer_wire_message(&current_peer_wire_message);

  close(fd);
  free(current_piece_state.piece_buffer);
  free(current_peers);
  free_bencode_object(&response_obj);
  free(tracker_response.data);
  free_bencode_object(&obj);
  free_buffer(&buffer);

  return 0;
}
