#include "munit.h"
#include "peer_connection.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static MunitResult test_receive_peer_wire_message_returns_interested_message(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    return MUNIT_FAIL;
  }

  unsigned char send_interest_bytes[] = {0x00, 0x00, 0x00, 0x01, 0x02};

  ssize_t sent =
      send(sockets[0], send_interest_bytes, sizeof(send_interest_bytes), 0);

  if (sent != sizeof(send_interest_bytes)) {
    close(sockets[0]);
    close(sockets[1]);
    return MUNIT_FAIL;
  }

  peer_wire_message_t receive_msg = {.message_id = PEER_MESSAGE_INVALID};

  bool received = receive_peer_wire_message(sockets[1], &receive_msg);

  munit_assert_true(received);
  munit_assert_int(receive_msg.message_id, ==, PEER_MESSAGE_INTERESTED);
  munit_assert_null(receive_msg.payload);
  munit_assert_false(receive_msg.is_keep_alive);
  munit_assert_size(receive_msg.payload_length, ==, 0);

  close(sockets[0]);
  close(sockets[1]);
  free_peer_wire_message(&receive_msg);

  return MUNIT_OK;
}

static MunitResult test_receive_peer_wire_message_returns_have_message(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    return MUNIT_FAIL;
  }

  unsigned char send_have_bytes[] = {0x00, 0x00, 0x00, 0x05, 0x04,
                                     0x00, 0x00, 0x00, 0x07};

  ssize_t sent = send(sockets[0], send_have_bytes, sizeof(send_have_bytes), 0);

  if (sent != sizeof(send_have_bytes)) {
    close(sockets[0]);
    close(sockets[1]);
    return MUNIT_FAIL;
  }

  peer_wire_message_t receive_msg = {.message_id = PEER_MESSAGE_INVALID};

  bool received = receive_peer_wire_message(sockets[1], &receive_msg);

  munit_assert_true(received);
  munit_assert_int(receive_msg.message_id, ==, PEER_MESSAGE_HAVE);
  munit_assert_false(receive_msg.is_keep_alive);

  unsigned char expected_payload[] = {0x00, 0x00, 0x00, 0x07};
  size_t expected_payload_length = sizeof(expected_payload);

  munit_assert_size(receive_msg.payload_length, ==, expected_payload_length);
  munit_assert_not_null(receive_msg.payload);
  munit_assert_memory_equal(expected_payload_length, expected_payload,
                            receive_msg.payload);

  close(sockets[0]);
  close(sockets[1]);
  free_peer_wire_message(&receive_msg);

  return MUNIT_OK;
}

static MunitResult test_receive_peer_wire_message_returns_keep_alive_message(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    return MUNIT_FAIL;
  }

  unsigned char send_keep_alive_bytes[] = {0x00, 0x00, 0x00, 0x00};

  ssize_t sent =
      send(sockets[0], send_keep_alive_bytes, sizeof(send_keep_alive_bytes), 0);

  if (sent != sizeof(send_keep_alive_bytes)) {
    close(sockets[0]);
    close(sockets[1]);
    return MUNIT_FAIL;
  }

  peer_wire_message_t receive_msg = {.message_id = PEER_MESSAGE_INVALID};

  bool received = receive_peer_wire_message(sockets[1], &receive_msg);

  munit_assert_true(received);
  munit_assert_int(receive_msg.message_id, ==, PEER_MESSAGE_INVALID);
  munit_assert_true(receive_msg.is_keep_alive);
  munit_assert_size(receive_msg.payload_length, ==, 0);
  munit_assert_null(receive_msg.payload);

  close(sockets[0]);
  close(sockets[1]);
  free_peer_wire_message(&receive_msg);

  return MUNIT_OK;
}

static MunitResult
test_receive_peer_wire_message_rejects_unsupported_message_id(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    return MUNIT_FAIL;
  }

  unsigned char send_unsupported_message_id_bytes[] = {
      0x00, 0x00, 0x00, 0x05, 0xa7, 0x00, 0x00, 0x00, 0x07};

  ssize_t sent = send(sockets[0], send_unsupported_message_id_bytes,
                      sizeof(send_unsupported_message_id_bytes), 0);

  if (sent != sizeof(send_unsupported_message_id_bytes)) {
    close(sockets[0]);
    close(sockets[1]);
    return MUNIT_FAIL;
  }

  peer_wire_message_t receive_msg = {.message_id = PEER_MESSAGE_INVALID};

  bool received = receive_peer_wire_message(sockets[1], &receive_msg);

  munit_assert_false(received);
  munit_assert_int(receive_msg.message_id, ==, PEER_MESSAGE_INVALID);
  munit_assert_false(receive_msg.is_keep_alive);

  munit_assert_size(receive_msg.payload_length, ==, 0);
  munit_assert_null(receive_msg.payload);

  close(sockets[0]);
  close(sockets[1]);

  return MUNIT_OK;
}

static MunitResult
test_receive_peer_wire_message_rejects_wrong_declared_payload_length(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    return MUNIT_FAIL;
  }

  unsigned char send_wrong_payload_length_for_unchoke[] = {0x00, 0x00, 0x00,
                                                           0x09, 0x01};

  ssize_t sent = send(sockets[0], send_wrong_payload_length_for_unchoke,
                      sizeof(send_wrong_payload_length_for_unchoke), 0);

  if (sent != sizeof(send_wrong_payload_length_for_unchoke)) {
    close(sockets[0]);
    close(sockets[1]);
    return MUNIT_FAIL;
  }

  peer_wire_message_t receive_msg = {.message_id = PEER_MESSAGE_INVALID};

  bool received = receive_peer_wire_message(sockets[1], &receive_msg);

  munit_assert_false(received);

  close(sockets[0]);
  close(sockets[1]);

  return MUNIT_OK;
}

static MunitResult
test_receive_peer_wire_message_rejects_incomplete_send_payload(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    return MUNIT_FAIL;
  }

  unsigned char send_incomplete_payload_bytes[] = {0x00, 0x00, 0x00, 0x05,
                                                   0x04, 0x00, 0x00};

  ssize_t sent = send(sockets[0], send_incomplete_payload_bytes,
                      sizeof(send_incomplete_payload_bytes), 0);

  if (sent != sizeof(send_incomplete_payload_bytes)) {
    close(sockets[0]);
    close(sockets[1]);
    return MUNIT_FAIL;
  }

  close(sockets[0]);

  peer_wire_message_t receive_msg = {.message_id = PEER_MESSAGE_INVALID};

  bool received = receive_peer_wire_message(sockets[1], &receive_msg);

  munit_assert_false(received);

  close(sockets[1]);

  return MUNIT_OK;
}

static MunitResult test_receive_peer_wire_message_rejects_unresetted_message(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    return MUNIT_FAIL;
  }

  unsigned char send_have_bytes[] = {0x00, 0x00, 0x00, 0x05, 0x04,
                                     0x00, 0x00, 0x00, 0x07};

  ssize_t sent = send(sockets[0], send_have_bytes, sizeof(send_have_bytes), 0);

  if (sent != sizeof(send_have_bytes)) {
    close(sockets[0]);
    close(sockets[1]);
    return MUNIT_FAIL;
  }

  peer_wire_message_t unresetted_msg = {.message_id = PEER_MESSAGE_CHOKE};

  bool received = receive_peer_wire_message(sockets[1], &unresetted_msg);

  munit_assert_false(received);

  close(sockets[0]);
  close(sockets[1]);

  return MUNIT_OK;
}
static MunitResult
test_bitfield_applied_applies_bitfield(const MunitParameter params[],
                                       void *user_data) {

  (void)params;
  (void)user_data;

  peer_connection_t connection = {
      .peer_piece_bitfield = {.bytes = NULL, .length = 0}};
  const size_t piece_count = 10;
  const unsigned char bitfield_10_pieces[] = {0xB2, 0xC0};

  bool applied = bitfield_applied(&connection, bitfield_10_pieces,
                                  sizeof(bitfield_10_pieces), piece_count);

  munit_assert_true(applied);
  munit_assert_not_null(connection.peer_piece_bitfield.bytes);
  munit_assert_size(sizeof(bitfield_10_pieces), ==,
                    connection.peer_piece_bitfield.length);

  munit_assert_memory_equal(sizeof(bitfield_10_pieces), bitfield_10_pieces,
                            connection.peer_piece_bitfield.bytes);

  free(connection.peer_piece_bitfield.bytes);

  return MUNIT_OK;
}

static MunitResult test_bitfield_applied_rejects_wrong_payload_length(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  peer_connection_t connection = {
      .peer_piece_bitfield = {.bytes = NULL, .length = 0}};
  const size_t piece_count = 10;

  const unsigned char bitfield_10_pieces[] = {0xB2};
  const size_t wrong_payload_length = 1;

  bool applied = bitfield_applied(&connection, bitfield_10_pieces,
                                  wrong_payload_length, piece_count);

  munit_assert_false(applied);
  munit_assert_null(connection.peer_piece_bitfield.bytes);
  munit_assert_size(connection.peer_piece_bitfield.length, ==, 0);

  return MUNIT_OK;
}

static MunitResult
test_bitfield_applied_rejects_padding_bit_set(const MunitParameter params[],
                                              void *user_data) {

  (void)params;
  (void)user_data;

  peer_connection_t connection = {
      .peer_piece_bitfield = {.bytes = NULL, .length = 0}};
  const size_t piece_count = 10;

  const unsigned char bitfield_padding_set[] = {0x55, 0xFF};

  bool applied = bitfield_applied(&connection, bitfield_padding_set,
                                  sizeof(bitfield_padding_set), piece_count);

  munit_assert_false(applied);
  munit_assert_null(connection.peer_piece_bitfield.bytes);
  munit_assert_size(connection.peer_piece_bitfield.length, ==, 0);

  return MUNIT_OK;
}

static MunitResult test_bitfield_applied_rejects_already_applied_bitfield(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  unsigned char bitfield_10_pieces[] = {0xB2, 0xC0};
  unsigned char *ownership_test_pointer = bitfield_10_pieces;
  const size_t payload_length = sizeof(bitfield_10_pieces);
  const size_t piece_count = 10;

  peer_connection_t connection = {
      .peer_piece_bitfield = {.bytes = ownership_test_pointer,
                              .length = payload_length}};

  bool applied = bitfield_applied(&connection, bitfield_10_pieces,
                                  sizeof(bitfield_10_pieces), piece_count);

  munit_assert_false(applied);
  munit_assert_not_null(connection.peer_piece_bitfield.bytes);
  munit_assert_size(payload_length, ==, connection.peer_piece_bitfield.length);
  munit_assert_ptr(ownership_test_pointer, ==,
                   connection.peer_piece_bitfield.bytes);

  return MUNIT_OK;
}
static MunitResult
test_peer_send_request_sends_17_bytes(const MunitParameter params[],
                                      void *user_data) {

  (void)params;
  (void)user_data;

  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    return MUNIT_FAIL;
  }
  uint32_t piece_index = 2;
  uint32_t begin = 16384;
  uint32_t length = 16384;

  bool request_sent = peer_send_request(sockets[0], piece_index, begin, length);

  if (!request_sent) {
    close(sockets[0]);
    close(sockets[1]);
    return MUNIT_FAIL;
  }

  size_t received_total = 0;
  size_t expected_byte_count = PEER_MESSAGE_REQUEST_LENGTH;
  unsigned char receive_request_buffer[PEER_MESSAGE_REQUEST_LENGTH] = {0};

  while (received_total < expected_byte_count) {

    ssize_t received = recv(sockets[1], receive_request_buffer + received_total,
                            expected_byte_count - received_total, 0);

    if (received == 0) {
      close(sockets[0]);
      close(sockets[1]);
      return MUNIT_FAIL;
    }

    if (received == -1) {

      if (errno == EINTR) {
        continue;
      }

      close(sockets[0]);
      close(sockets[1]);
      return MUNIT_FAIL;
    }

    received_total += received;
  }

  unsigned char expected_receive_bytes[] = {0x00, 0x00, 0x00, 0x0D, 0x06, 0x00,
                                            0x00, 0x00, 0x02, 0x00, 0x00, 0x40,
                                            0x00, 0x00, 0x00, 0x40, 0x00};

  size_t expected_payload_length = PEER_MESSAGE_REQUEST_LENGTH;

  munit_assert_memory_equal(expected_payload_length, expected_receive_bytes,
                            receive_request_buffer);

  close(sockets[0]);
  close(sockets[1]);

  return MUNIT_OK;
}

static MunitResult
test_peer_send_request_rejects_invalid_socket(const MunitParameter params[],
                                              void *user_data) {

  (void)params;
  (void)user_data;

  int sockets[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
    return MUNIT_FAIL;
  }
  uint32_t piece_index = 2;
  uint32_t begin = 16384;
  uint32_t length = 16384;

  bool request_sent = peer_send_request(-1, piece_index, begin, length);

  munit_assert_false(request_sent);

  close(sockets[0]);
  close(sockets[1]);

  return MUNIT_OK;
}

static MunitResult
test_peer_send_interested_rejects_invalid_socket(const MunitParameter params[],
                                                 void *user_data) {
  (void)params;
  (void)user_data;

  bool request_sent = peer_send_interested(-1);

  munit_assert_false(request_sent);

  return MUNIT_OK;
}

static MunitResult
test_determine_piece_info_from_piece_payload_returns_correct_output(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  unsigned char payload_buffer[] = {0x00, 0x00, 0x00, 0x02, 0x00,
                                    0x00, 0x40, 0x00, 0xFF, 0xFF};

  size_t payload_length = sizeof(payload_buffer);

  peer_wire_message_t test_msg = {.message_id = PEER_MESSAGE_PIECE,
                                  .payload = payload_buffer,
                                  .payload_length = payload_length,
                                  .is_keep_alive = false};

  size_t piece_index = 0;
  size_t begin = 0;
  size_t block_length = 0;
  const unsigned char *block_buffer = NULL;

  bool determined = determine_piece_info_from_piece_payload(
      &test_msg, &piece_index, &begin, &block_buffer, &block_length);

  munit_assert_true(determined);

  size_t expected_piece_index = 2;
  size_t expected_begin = 16384;
  size_t expected_block_length = 2;

  munit_assert_size(expected_piece_index, ==, piece_index);
  munit_assert_size(expected_block_length, ==, block_length);
  munit_assert_size(expected_begin, ==, begin);
  munit_assert_memory_equal(expected_block_length, payload_buffer + 8,
                            block_buffer);
  munit_assert_ptr(payload_buffer + 8, ==, block_buffer);

  return MUNIT_OK;
}

static MunitResult
test_determine_piece_info_from_piece_payload_rejects_invalid_payload_length(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  unsigned char too_small_payload_buffer[] = {0x00, 0x00, 0x40,
                                              0x00, 0xFF, 0xFF};

  size_t payload_length = sizeof(too_small_payload_buffer);

  peer_wire_message_t test_msg = {.message_id = PEER_MESSAGE_PIECE,
                                  .payload = too_small_payload_buffer,
                                  .payload_length = payload_length,
                                  .is_keep_alive = false};

  size_t piece_index = 0;
  size_t begin = 0;
  size_t block_length = 0;
  const unsigned char *block_buffer = NULL;

  bool determined = determine_piece_info_from_piece_payload(
      &test_msg, &piece_index, &begin, &block_buffer, &block_length);

  munit_assert_false(determined);

  size_t expected_unchanged_piece_index = 0;
  size_t expected_unchanged_begin = 0;
  size_t expected_unchanged_block_length = 0;

  munit_assert_size(expected_unchanged_piece_index, ==, piece_index);
  munit_assert_size(expected_unchanged_block_length, ==, block_length);
  munit_assert_size(expected_unchanged_begin, ==, begin);
  munit_assert_null(block_buffer);

  return MUNIT_OK;
}

static MunitResult
test_determine_piece_info_from_piece_payload_rejects_wrong_message_id(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  unsigned char payload_buffer[] = {0x00, 0x00, 0x00, 0x02, 0x00,
                                    0x00, 0x40, 0x00, 0xFF, 0xFF};

  size_t payload_length = sizeof(payload_buffer);
  enum MessageId wrong_message_id = PEER_MESSAGE_INTERESTED;

  peer_wire_message_t test_msg = {.message_id = wrong_message_id,
                                  .payload = payload_buffer,
                                  .payload_length = payload_length,
                                  .is_keep_alive = false};

  size_t piece_index = 0;
  size_t begin = 0;
  size_t block_length = 0;
  const unsigned char *block_buffer = NULL;

  bool determined = determine_piece_info_from_piece_payload(
      &test_msg, &piece_index, &begin, &block_buffer, &block_length);

  munit_assert_false(determined);

  size_t expected_unchanged_piece_index = 0;
  size_t expected_unchanged_begin = 0;
  size_t expected_unchanged_block_length = 0;

  munit_assert_size(expected_unchanged_piece_index, ==, piece_index);
  munit_assert_size(expected_unchanged_block_length, ==, block_length);
  munit_assert_size(expected_unchanged_begin, ==, begin);
  munit_assert_null(block_buffer);

  return MUNIT_OK;
}

static MunitTest tests[] = {
    {"/receive-peer-wire-message/returns-interested",
     test_receive_peer_wire_message_returns_interested_message, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/receive-peer-wire-message/returns-have",
     test_receive_peer_wire_message_returns_have_message, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/receive-peer-wire-message/returns-keep-alive",
     test_receive_peer_wire_message_returns_keep_alive_message, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/receive-peer-wire-message/rejects-unsupported-message-id",
     test_receive_peer_wire_message_rejects_unsupported_message_id, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/receive-peer-wire-message/rejects-wrong-declared-payload-length",
     test_receive_peer_wire_message_rejects_wrong_declared_payload_length, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/receive-peer-wire-message/rejects-incomplete-payload",
     test_receive_peer_wire_message_rejects_incomplete_send_payload, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/receive-peer-wire-message/rejects-unresetted-message",
     test_receive_peer_wire_message_rejects_unresetted_message, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/bitfield_applied/applies-bitfield",
     test_bitfield_applied_applies_bitfield, NULL, NULL, MUNIT_TEST_OPTION_NONE,
     NULL},
    {"/bitfield_applied/rejects-wrong-payload-length",
     test_bitfield_applied_rejects_wrong_payload_length, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/bitfield_applied/rejects-padding-bit-set",
     test_bitfield_applied_rejects_padding_bit_set, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/bitfield_applied/rejects-already-set-bitfield",
     test_bitfield_applied_rejects_already_applied_bitfield, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/peer_send_request/returns-exactly-17-bytes",
     test_peer_send_request_sends_17_bytes, NULL, NULL, MUNIT_TEST_OPTION_NONE,
     NULL},
    {"/peer_send_request/rejects-invalid-socket",
     test_peer_send_request_rejects_invalid_socket, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/peer_send_interested/rejects-invalid-socket",
     test_peer_send_interested_rejects_invalid_socket, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/determine_piece_info/returns-correct-output",
     test_determine_piece_info_from_piece_payload_returns_correct_output, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/determine_piece_info/rejects-invalid-payload-length",
     test_determine_piece_info_from_piece_payload_rejects_invalid_payload_length,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/determine_piece_info/rejects-wrong-message-id",
     test_determine_piece_info_from_piece_payload_rejects_wrong_message_id,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite peer_connection_suite = {"/peer_connection", tests, NULL, 1,
                                          MUNIT_SUITE_OPTION_NONE};
