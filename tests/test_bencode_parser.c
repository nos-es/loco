#include "bencode_parser.h"
#include "bencode_types.h"
#include "munit.h"
#include <stddef.h>
#include <stdio.h>

static MunitResult
test_bencode_parser_init_accepts_valid_buffer(const MunitParameter params[],
                                              void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  unsigned char expected_data[7] = "4:spam";
  // input_length = How many bytes should be parsed. Can also be less then
  // actual buffer length. When the parser should only parse certain bytes.
  size_t input_length = sizeof(expected_data) - 1;
  bool parser_initialzed =
      bencode_parser_init(&parser, expected_data, input_length);

  munit_assert_true(parser_initialzed);
  munit_assert_ptr_equal(parser.data, expected_data);
  munit_assert_size(parser.length, ==, input_length);
  munit_assert_size(parser.position, ==, 0);
  return MUNIT_OK;
}

static MunitResult
test_bencode_parser_init_rejects_null_data_with_nonzero_length(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  bool parser_initialzed = bencode_parser_init(&parser, NULL, 6);
  munit_assert_false(parser_initialzed);
  return MUNIT_OK;
}

static MunitResult
test_parse_bencode_buffer_parses_byte_string_and_advances_position(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "4:spam";
  size_t input_length = sizeof(input_data) - 1;
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t expected_parser_position_after_parse = 6;
  bencode_object_t parsed_obj = {
      .type = INVALID, .value.byte_string = {.data = NULL, .length = 0}};
  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);

  return MUNIT_OK;
}

static MunitResult
test_parse_bencode_buffer_resets_parser_position_when_parse_failed(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "5:spam";
  size_t input_length = sizeof(input_data) - 1;
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t expected_parser_position_after_parse = 0;

  bencode_object_t parsed_obj = {
      .type = INVALID, .value.byte_string = {.data = NULL, .length = 0}};
  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);

  return MUNIT_OK;
}

static MunitResult test_parse_bencode_buffer_returns_byte_string_object(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "4:spam";
  size_t input_length = sizeof(input_data) - 1;
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {
      .type = INVALID, .value.byte_string = {.data = NULL, .length = 0}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_int(parsed_obj.type, ==, BYTE_STRING);
  munit_assert_size(parsed_obj.value.byte_string.length, ==, 4);
  munit_assert_ptr_equal(parsed_obj.value.byte_string.data, input_data + 2);

  const unsigned char expected_string[] = "spam";
  munit_assert_memory_equal(4, expected_string,
                            parsed_obj.value.byte_string.data);

  return MUNIT_OK;
}

static MunitResult test_parse_bencode_buffer_leaves_output_unchanged_on_failure(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "5:spam";
  size_t input_length = sizeof(input_data) - 1;
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);
  unsigned char dummy_data[] = "dummy data";
  size_t dummy_len = 999;

  bencode_object_t parsed_obj = {
      .type = BYTE_STRING,
      .value.byte_string = {.data = dummy_data, .length = dummy_len}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_int(parsed_obj.type, ==, BYTE_STRING);
  munit_assert_size(parsed_obj.value.byte_string.length, ==, dummy_len);
  munit_assert_ptr_equal(parsed_obj.value.byte_string.data, dummy_data);

  munit_assert_memory_equal(10, dummy_data, parsed_obj.value.byte_string.data);

  return MUNIT_OK;
}

static MunitResult
test_parse_bencode_buffer_returns_integer_object(const MunitParameter params[],
                                                 void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i42e";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 4;
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {.type = INTEGER, .value.integer = 0};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_int(parsed_obj.type, ==, INTEGER);
  munit_assert_int64(parsed_obj.value.integer, ==, 42);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);

  return MUNIT_OK;
}
static MunitResult
test_parse_bencode_buffer_returns_integer_object_for_zero_integer(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i0e";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 3;
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {.type = INTEGER, .value.integer = 99};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_int(parsed_obj.type, ==, INTEGER);
  munit_assert_int64(parsed_obj.value.integer, ==, 0);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);

  return MUNIT_OK;
}

static MunitResult
test_parse_bencode_buffer_returns_returns_false_for_leading_zero_integer(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i03e";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 0;
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {.type = INTEGER, .value.integer = 99};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);

  return MUNIT_OK;
}
static MunitResult
test_parse_bencode_buffer_returns_returns_false_for_empty_integer_syntax(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "ie";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 0;
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {.type = INTEGER, .value.integer = 99};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);

  return MUNIT_OK;
}
static MunitResult test_parse_bencode_buffer_rejects_integer_without_terminator(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i42";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 0;
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {.type = INTEGER, .value.integer = 99};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);

  return MUNIT_OK;
}

static MunitResult
test_parse_bencode_buffer_accepts_int64_max_value(const MunitParameter params[],
                                                  void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i9223372036854775807e";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;
  printf("expected_parser_position_after_parse: %zu\n",
         expected_parser_position_after_parse);
  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {.type = INTEGER, .value.integer = 99};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);

  return MUNIT_OK;
}
static MunitTest tests[] = {
    {"/init/accepts-valid-buffer",
     test_bencode_parser_init_accepts_valid_buffer, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/init/rejects-null-data-with-nonzero-length",
     test_bencode_parser_init_rejects_null_data_with_nonzero_length, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/byte-string/advances-position",
     test_parse_bencode_buffer_parses_byte_string_and_advances_position, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/byte-string/restores-position-on-incomplete-input",
     test_parse_bencode_buffer_resets_parser_position_when_parse_failed, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/object/output-unchanged-on-failure",
     test_parse_bencode_buffer_leaves_output_unchanged_on_failure, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/object/returns-byte-string-object",
     test_parse_bencode_buffer_returns_byte_string_object, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/object/returns-integer-object",
     test_parse_bencode_buffer_returns_integer_object, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/object/returns-zero-integer-object",
     test_parse_bencode_buffer_returns_integer_object_for_zero_integer, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/object/returns-false-leading-zero",
     test_parse_bencode_buffer_returns_returns_false_for_leading_zero_integer,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/object/returns-false-empty-integer",
     test_parse_bencode_buffer_returns_returns_false_for_empty_integer_syntax,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/integer/rejects-missing-terminator",
     test_parse_bencode_buffer_rejects_integer_without_terminator, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/integer/accepts_int64_max",
     test_parse_bencode_buffer_accepts_int64_max_value, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

static const MunitSuite suite = {"/bencode_parser", tests, NULL, 1,
                                 MUNIT_SUITE_OPTION_NONE};

int main(int argc, char *argv[]) {
  return munit_suite_main(&suite, NULL, argc, argv);
}
