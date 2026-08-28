#include "info_hash.h"
#include "munit.h"
#include <stddef.h>
#include <stdint.h>

static MunitResult test_info_hash_compute_info_hash_returns_correct_hash(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  unsigned char input_data[] = "abc";
  const unsigned char expected_sha1[] = {
      0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e,
      0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d};

  size_t sha1_length = sizeof(expected_sha1);
  size_t input_length = sizeof(input_data) - 1;

  info_hash_t info_hash = {.bytes = {0}};

  bool computed = compute_info_hash(input_data, input_length, &info_hash);

  munit_assert_true(computed);
  munit_assert_memory_equal(sha1_length, info_hash.bytes, expected_sha1);

  return MUNIT_OK;
}

static MunitTest tests[] = {
    {"/compute_info_hash/computes-sha1",
     test_info_hash_compute_info_hash_returns_correct_hash, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite info_hash_suite = {"/info_hash", tests, NULL, 1,
                                    MUNIT_SUITE_OPTION_NONE};
