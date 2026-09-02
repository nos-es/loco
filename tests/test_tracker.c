#include "bencode_types.h"
#include "munit.h"
#include "tracker.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static MunitResult
test_build_tracker_url_returns_correct_url(const MunitParameter params[],
                                           void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char announce_bytes[] = "https://tracker.example/announce";
  bencode_segment_t announce = {.data = announce_bytes,
                                .length = sizeof(announce_bytes) - 1};

  tracker_request_t tracker_req = {
      .info_hash = {.bytes = {0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f, 0x41,
                              0x5a, 0x61, 0x7a, 0x7f, 0x80, 0xa9, 0xff,
                              0x10, 0x2d, 0x5f, 0x2e, 0x7e, 0x30}},
      .peer_id = {.bytes = {'-',  'L',  'O',  '0',  '0',  '0',  '1',
                            '-',  0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f,
                            0x41, 0x7a, 0x80, 0xa9, 0xff, 0x5f}},
      .port = 1234,
      .uploaded = 0,
      .downloaded = 0,
      .left = 100,
      .compact = true};

  const char *expected_url =
      "https://tracker.example/"
      "announce?info_hash=%00%01%20%25%2F%3FAZaz%7F%80%A9%FF%"
      "10-_.~0&peer_id=-LO0001-%00%01%20%25%2F%3FAz%80%A9%FF_&"
      "port=1234&uploaded=0&downloaded=0&left=100&compact=1";

  char *result = build_tracker_url(&announce, &tracker_req);
  munit_assert_not_null(result);
  munit_assert_string_equal(expected_url, result);
  free(result);

  return MUNIT_OK;
}

static MunitResult
test_build_tracker_url_returns_correct_url_with_questionmark_in_announce(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char announce_bytes[] =
      "https://tracker.example/announce?token=abc";
  bencode_segment_t announce = {.data = announce_bytes,
                                .length = sizeof(announce_bytes) - 1};

  tracker_request_t tracker_req = {
      .info_hash = {.bytes = {0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f, 0x41,
                              0x5a, 0x61, 0x7a, 0x7f, 0x80, 0xa9, 0xff,
                              0x10, 0x2d, 0x5f, 0x2e, 0x7e, 0x30}},
      .peer_id = {.bytes = {'-',  'L',  'O',  '0',  '0',  '0',  '1',
                            '-',  0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f,
                            0x41, 0x7a, 0x80, 0xa9, 0xff, 0x5f}},
      .port = 1234,
      .uploaded = 0,
      .downloaded = 0,
      .left = 100,
      .compact = true};

  const char *expected_url =
      "https://tracker.example/"
      "announce?token=abc&info_hash=%00%01%20%25%2F%3FAZaz%7F%80%A9%FF%"
      "10-_.~0&peer_id=-LO0001-%00%01%20%25%2F%3FAz%80%A9%FF_&"
      "port=1234&uploaded=0&downloaded=0&left=100&compact=1";

  char *result = build_tracker_url(&announce, &tracker_req);
  munit_assert_not_null(result);
  munit_assert_string_equal(expected_url, result);
  free(result);

  return MUNIT_OK;
}

static MunitResult
test_build_tracker_url_returns_correct_url_with_event_query_parameter(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char announce_bytes[] = "https://tracker.example/announce";
  bencode_segment_t announce = {.data = announce_bytes,
                                .length = sizeof(announce_bytes) - 1};

  tracker_request_t tracker_req = {
      .info_hash = {.bytes = {0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f, 0x41,
                              0x5a, 0x61, 0x7a, 0x7f, 0x80, 0xa9, 0xff,
                              0x10, 0x2d, 0x5f, 0x2e, 0x7e, 0x30}},
      .peer_id = {.bytes = {'-',  'L',  'O',  '0',  '0',  '0',  '1',
                            '-',  0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f,
                            0x41, 0x7a, 0x80, 0xa9, 0xff, 0x5f}},
      .port = 1234,
      .uploaded = 0,
      .downloaded = 0,
      .left = 100,
      .compact = true,
      .event = TRACKER_EVENT_STARTED};

  const char *expected_url =
      "https://tracker.example/"
      "announce?info_hash=%00%01%20%25%2F%3FAZaz%7F%80%A9%FF%"
      "10-_.~0&peer_id=-LO0001-%00%01%20%25%2F%3FAz%80%A9%FF_&"
      "port=1234&uploaded=0&downloaded=0&left=100&compact=1&event=started";

  char *result = build_tracker_url(&announce, &tracker_req);

  munit_assert_not_null(result);
  munit_assert_string_equal(expected_url, result);
  free(result);

  return MUNIT_OK;
}

static MunitResult
test_build_tracker_url_rejects_unsupported_event(const MunitParameter params[],
                                                 void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char announce_bytes[] = "https://tracker.example/announce";
  bencode_segment_t announce = {.data = announce_bytes,
                                .length = sizeof(announce_bytes) - 1};

  tracker_request_t tracker_req = {
      .info_hash = {.bytes = {0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f, 0x41,
                              0x5a, 0x61, 0x7a, 0x7f, 0x80, 0xa9, 0xff,
                              0x10, 0x2d, 0x5f, 0x2e, 0x7e, 0x30}},
      .peer_id = {.bytes = {'-',  'L',  'O',  '0',  '0',  '0',  '1',
                            '-',  0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f,
                            0x41, 0x7a, 0x80, 0xa9, 0xff, 0x5f}},
      .port = 1234,
      .uploaded = 0,
      .downloaded = 0,
      .left = 100,
      .compact = true,
      .event = 42};

  char *result = build_tracker_url(&announce, &tracker_req);
  munit_assert_null(result);

  return MUNIT_OK;
}
static MunitResult
test_write_chunks_to_tracker_response_buffer_writes_chunks_correctly(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  tracker_response_buffer_t response = {.data = NULL, .length = 0};

  // First chunk
  size_t written_bytes =
      write_chunk_to_tracker_response_buffer("abc", 1, 3, &response);

  munit_assert_size(written_bytes, ==, 3);
  munit_assert_size(response.length, ==, 3);

  // Second chunk
  unsigned char chunk[] = {0x00, 0xff};

  written_bytes =
      write_chunk_to_tracker_response_buffer((char *)chunk, 1, 2, &response);

  munit_assert_size(written_bytes, ==, 2);
  munit_assert_size(response.length, ==, 5);

  // Third chunk
  written_bytes =
      write_chunk_to_tracker_response_buffer("xyz", 1, 3, &response);

  munit_assert_size(written_bytes, ==, 3);
  munit_assert_size(response.length, ==, 8);

  const unsigned char expected[] = {'a', 'b', 'c', 0x00, 0xff, 'x', 'y', 'z'};

  munit_assert_memory_equal(sizeof(expected), expected, response.data);

  free(response.data);

  return MUNIT_OK;
}

static MunitResult
test_write_chunks_to_tracker_response_buffer_rejects_null_response(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  size_t written_bytes =
      write_chunk_to_tracker_response_buffer("abc", 1, 3, NULL);

  munit_assert_size(written_bytes, ==, 0);

  return MUNIT_OK;
}

static MunitResult
test_tracker_announce_writes_bencode_in_tracker_response_buffer(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;
  const unsigned char announce_url[] = "http://127.0.0.1:8000/announce";
  const bencode_segment_t announce = {.data = announce_url,
                                      .length = sizeof(announce_url) - 1};

  const tracker_request_t tracker_req = {
      .info_hash = {.bytes = {0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f, 0x41,
                              0x5a, 0x61, 0x7a, 0x7f, 0x80, 0xa9, 0xff,
                              0x10, 0x2d, 0x5f, 0x2e, 0x7e, 0x30}},
      .peer_id = {.bytes = {'-',  'L',  'O',  '0',  '0',  '0',  '1',
                            '-',  0x00, 0x01, 0x20, 0x25, 0x2f, 0x3f,
                            0x41, 0x7a, 0x80, 0xa9, 0xff, 0x5f}},
      .port = 1234,
      .uploaded = 0,
      .downloaded = 0,
      .left = 100,
      .compact = true,
      .event = TRACKER_EVENT_STARTED};
  tracker_response_buffer_t response = {.data = NULL, .length = 0};

  bool announced = tracker_announce(&announce, &tracker_req, &response);

  munit_assert_true(announced);

  const char expected_response[] = "d8:intervali1800e5:peers0:e";
  const size_t expected_response_size = sizeof(expected_response) - 1;
  munit_assert_memory_equal(expected_response_size, expected_response,
                            response.data);
  munit_assert_size(expected_response_size, ==, response.length);

  free(response.data);

  return MUNIT_OK;
}
static MunitTest tests[] = {
    {"/build_tracker_url/returns-correct-url",
     test_build_tracker_url_returns_correct_url, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/build_tracker_url/returns-correct-url-questionmark-announce",
     test_build_tracker_url_returns_correct_url_with_questionmark_in_announce,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/build_tracker_url/returns-correct-url-event-parameter",
     test_build_tracker_url_returns_correct_url_with_event_query_parameter,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/build_tracker_url/rejects-unsupported-event",
     test_build_tracker_url_rejects_unsupported_event, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/write_chunks/writes-chunks-correctly",
     test_write_chunks_to_tracker_response_buffer_writes_chunks_correctly, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/write_chunks/rejects-null-response",
     test_write_chunks_to_tracker_response_buffer_rejects_null_response, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/announce_tracker/returns-response",
     test_tracker_announce_writes_bencode_in_tracker_response_buffer, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite tracker_suite = {"/tracker", tests, NULL, 1,
                                  MUNIT_SUITE_OPTION_NONE};
