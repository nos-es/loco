#include "bencode_types.h"
#include "munit.h"
#include "tracker.h"
#include <stddef.h>
#include <stdint.h>

static MunitResult test_build_tracker_url(const MunitParameter params[],
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

  char *result = build_tracker_url(&announce, &tracker_req);
  munit_assert_not_null(result);
  free(result);

  return MUNIT_OK;
}

static MunitTest tests[] = {
    {"/build_tracker/build_tracker", test_build_tracker_url, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite tracker_suite = {"/tracker", tests, NULL, 1,
                                  MUNIT_SUITE_OPTION_NONE};
