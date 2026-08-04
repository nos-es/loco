#include "bencode_parser.h"
#include "munit.h"
#include <stddef.h>

MunitResult parser_init(const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  unsigned char expected_data[7] = "4:spam";
  size_t expected_length = 6;
  bool parser_initialzed =
      bencode_parser_init(&parser, expected_data, expected_length);

  munit_assert_true(parser_initialzed);
  munit_assert_ptr_equal(parser.data, expected_data);
  munit_assert_size(parser.length, ==, expected_length);
  munit_assert_size(parser.position, ==, 0);
  return MUNIT_OK;
}
MunitResult reject_null_data(const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  bool parser_initialzed = bencode_parser_init(&parser, NULL, 6);
  munit_assert_false(parser_initialzed);
  return MUNIT_OK;
}

static MunitTest tests[] = {
    {"/init_parser", parser_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/init_parser_reject_null_data", reject_null_data, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

static const MunitSuite suite = {"/bencode_parser_tests", tests, NULL, 1,
                                 MUNIT_SUITE_OPTION_NONE};

int main(int argc, char *argv[]) {
  return munit_suite_main(&suite, NULL, argc, argv);
}
