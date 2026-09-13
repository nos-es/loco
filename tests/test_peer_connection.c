#include "munit.h"
#include "peer_connection.h"
#include <stddef.h>
#include <stdint.h>
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
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite peer_connection_suite = {"/peer_connection", tests, NULL, 1,
                                          MUNIT_SUITE_OPTION_NONE};
