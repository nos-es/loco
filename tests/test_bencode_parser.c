#include "bencode_parser.h"
#include "bencode_types.h"
#include "munit.h"
#include <stddef.h>
#include <stdint.h>
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
  munit_assert_size(parsed_obj.start_offset, ==, 0);
  munit_assert_size(parsed_obj.encoded_length, ==, 4);
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

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {.type = INTEGER, .value.integer = 99};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, INTEGER);
  munit_assert_int64(parsed_obj.value.integer, ==, INT64_MAX);

  return MUNIT_OK;
}

static MunitResult test_parse_bencode_buffer_rejects_integer_overflow(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i9223372036854775808e";
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

static MunitResult test_parse_bencode_buffer_parse_parses_negative_integer(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i-42e";
  const int64_t expected_integer_value = -42;
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {.type = INTEGER, .value.integer = 99};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int64(parsed_obj.value.integer, ==, expected_integer_value);
  munit_assert_int(parsed_obj.type, ==, INTEGER);

  return MUNIT_OK;
}
static MunitResult
test_parse_bencode_buffer_parse_rejects_empty_negative_integer_syntax(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i-e";
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
test_parse_bencode_buffer_rejects_negative_zero(const MunitParameter params[],
                                                void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i-0e";
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
test_parse_bencode_buffer_accept_INT64_MIN(const MunitParameter params[],
                                           void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i-9223372036854775808e";
  const int64_t expected_integer_value = INT64_MIN;
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {.type = INTEGER, .value.integer = 99};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int64(parsed_obj.value.integer, ==, expected_integer_value);
  munit_assert_int(parsed_obj.type, ==, INTEGER);

  return MUNIT_OK;
}

static MunitResult
test_parse_bencode_buffer_rejects_below_INT64_MIN(const MunitParameter params[],
                                                  void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "i-9223372036854775809e";
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
test_parse_bencode_buffer_parses_empty_list(const MunitParameter params[],
                                            void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "le";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.list = {.items = NULL, .count = 99, .capacity = 123}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, LIST);
  munit_assert_ptr(parsed_obj.value.list.items, ==, NULL);
  munit_assert_size(parsed_obj.value.list.capacity, ==, 0);
  munit_assert_size(parsed_obj.value.list.count, ==, 0);

  return MUNIT_OK;
}
static MunitResult test_parse_bencode_buffer_parses_single_integer_in_list(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "li42ee";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.list = {.items = NULL, .count = 99, .capacity = 123}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, LIST);
  munit_assert_ptr(parsed_obj.value.list.items, !=, NULL);
  munit_assert_size(parsed_obj.value.list.capacity, ==, 1);
  munit_assert_size(parsed_obj.value.list.count, ==, 1);
  munit_assert_int(parsed_obj.value.list.items[0].type, ==, INTEGER);
  munit_assert_int64(parsed_obj.value.list.items[0].value.integer, ==, 42);
  free(parsed_obj.value.list.items);

  return MUNIT_OK;
}

static MunitResult test_parse_bencode_buffer_parses_multiple_integer_in_list(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "li1ei2ee";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.list = {.items = NULL, .count = 99, .capacity = 123}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, LIST);
  munit_assert_size(parsed_obj.value.list.count, ==, 2);
  munit_assert_ptr(parsed_obj.value.list.items, !=, NULL);
  munit_assert_int(parsed_obj.value.list.items[0].type, ==, INTEGER);
  munit_assert_int64(parsed_obj.value.list.items[0].value.integer, ==, 1);
  munit_assert_int(parsed_obj.value.list.items[1].type, ==, INTEGER);
  munit_assert_int64(parsed_obj.value.list.items[1].value.integer, ==, 2);
  munit_assert_true(parsed_obj.value.list.capacity >=
                    parsed_obj.value.list.count);
  free(parsed_obj.value.list.items);

  return MUNIT_OK;
}

static MunitResult test_parse_bencode_buffer_parses_mixed_elements_in_list(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "li1e4:spame";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;
  const unsigned char expected_string[] = "spam";

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.list = {.items = NULL, .count = 99, .capacity = 123}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, LIST);
  munit_assert_size(parsed_obj.value.list.count, ==, 2);
  munit_assert_ptr(parsed_obj.value.list.items, !=, NULL);
  munit_assert_int(parsed_obj.value.list.items[0].type, ==, INTEGER);
  munit_assert_int64(parsed_obj.value.list.items[0].value.integer, ==, 1);
  munit_assert_int(parsed_obj.value.list.items[1].type, ==, BYTE_STRING);
  munit_assert_size(parsed_obj.value.list.items[1].value.byte_string.length, ==,
                    4);
  munit_assert_ptr_equal(parsed_obj.value.list.items[1].value.byte_string.data,
                         input_data + 6);

  munit_assert_memory_equal(
      4, expected_string,
      parsed_obj.value.list.items[1].value.byte_string.data);
  munit_assert_true(parsed_obj.value.list.capacity >=
                    parsed_obj.value.list.count);
  free(parsed_obj.value.list.items);

  return MUNIT_OK;
}

static MunitResult
test_parse_bencode_buffer_parses_inner_lists(const MunitParameter params[],
                                             void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "lli1eee";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.list = {.items = NULL, .count = 99, .capacity = 123}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, LIST);
  munit_assert_size(parsed_obj.value.list.count, ==, 1);
  munit_assert_ptr(parsed_obj.value.list.items, !=, NULL);
  munit_assert_true(parsed_obj.value.list.capacity >=
                    parsed_obj.value.list.count);

  munit_assert_ptr(parsed_obj.value.list.items[0].value.list.items, !=, NULL);
  munit_assert_int(parsed_obj.value.list.items[0].type, ==, LIST);
  munit_assert_size(parsed_obj.value.list.items[0].value.list.count, ==, 1);
  munit_assert_true(parsed_obj.value.list.items[0].value.list.capacity >=
                    parsed_obj.value.list.items[0].value.list.count);

  munit_assert_int(parsed_obj.value.list.items[0].value.list.items[0].type, ==,
                   INTEGER);
  munit_assert_int64(
      parsed_obj.value.list.items[0].value.list.items[0].value.integer, ==, 1);

  free(parsed_obj.value.list.items[0].value.list.items); // inner list
  free(parsed_obj.value.list.items);                     // outer list

  return MUNIT_OK;
}

static MunitResult
test_free_bencode_buffer_parses_frees_and_resets_inner_elements_correctly(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "lli1eee";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.list = {.items = NULL, .count = 99, .capacity = 123}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, LIST);
  munit_assert_size(parsed_obj.value.list.count, ==, 1);
  munit_assert_ptr(parsed_obj.value.list.items, !=, NULL);
  munit_assert_true(parsed_obj.value.list.capacity >=
                    parsed_obj.value.list.count);

  munit_assert_ptr(parsed_obj.value.list.items[0].value.list.items, !=, NULL);
  munit_assert_int(parsed_obj.value.list.items[0].type, ==, LIST);
  munit_assert_size(parsed_obj.value.list.items[0].value.list.count, ==, 1);
  munit_assert_true(parsed_obj.value.list.items[0].value.list.capacity >=
                    parsed_obj.value.list.items[0].value.list.count);

  munit_assert_int(parsed_obj.value.list.items[0].value.list.items[0].type, ==,
                   INTEGER);
  munit_assert_int64(
      parsed_obj.value.list.items[0].value.list.items[0].value.integer, ==, 1);

  free_bencode_object(&parsed_obj);

  munit_assert_int(parsed_obj.type, ==, INVALID);
  munit_assert_size(parsed_obj.value.list.count, ==, 0);
  munit_assert_size(parsed_obj.value.list.capacity, ==, 0);
  munit_assert_ptr(parsed_obj.value.list.items, ==, NULL);
  return MUNIT_OK;
}

static MunitResult
test_free_bencode_buffer_reject_partially_parsed_list_and_cleans_ups(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "lli1eex";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 0;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {.type = INVALID,
                                 .value.list = {.items = NULL,
                                                .count = dummy_count,
                                                .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, INVALID);
  munit_assert_size(parsed_obj.value.list.count, ==, dummy_count);
  munit_assert_ptr(parsed_obj.value.list.items, ==, NULL);
  munit_assert_size(parsed_obj.value.list.capacity, ==, dummy_capacity);

  free_bencode_object(&parsed_obj);

  return MUNIT_OK;
}

static MunitResult
test_bencode_buffer_parses_empty_dictionary(const MunitParameter params[],
                                            void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "de";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, DICTIONARY);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, 0);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, ==, NULL);
  munit_assert_size(parsed_obj.value.dictionary.capacity, ==, 0);

  return MUNIT_OK;
}

static MunitResult test_bencode_buffer_parses_dictionary_with_single_element(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "d3:fooi42ee";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;
  const unsigned char expected_key[] = "foo";

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, DICTIONARY);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, 1);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, !=, NULL);
  munit_assert_size(parsed_obj.value.dictionary.capacity, ==, 1);

  munit_assert_int(parsed_obj.value.dictionary.entries[0].value.type, ==,
                   INTEGER);
  munit_assert_int64(parsed_obj.value.dictionary.entries[0].value.value.integer,
                     ==, 42);

  munit_assert_memory_equal(3, expected_key,
                            parsed_obj.value.dictionary.entries[0].key.data);
  munit_assert_size(parsed_obj.value.dictionary.entries[0].key.length, ==, 3);

  munit_assert_ptr_equal(parsed_obj.value.dictionary.entries[0].key.data,
                         input_data + 3);

  free_bencode_object(&parsed_obj);

  return MUNIT_OK;
}
static MunitResult test_bencode_buffer_parses_dictionary_with_two_elements(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "d3:bari7e3:fooi42ee";

  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;
  const unsigned char expected_first_key[] = "bar";
  const unsigned char expected_second_key[] = "foo";

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, DICTIONARY);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, 2);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, !=, NULL);
  munit_assert_true(parsed_obj.value.dictionary.capacity >=
                    parsed_obj.value.dictionary.count);

  munit_assert_int(parsed_obj.value.dictionary.entries[0].value.type, ==,
                   INTEGER);
  munit_assert_int64(parsed_obj.value.dictionary.entries[0].value.value.integer,
                     ==, 7);

  munit_assert_memory_equal(3, expected_first_key,
                            parsed_obj.value.dictionary.entries[0].key.data);

  munit_assert_size(parsed_obj.value.dictionary.entries[0].key.length, ==, 3);

  munit_assert_ptr_equal(parsed_obj.value.dictionary.entries[0].key.data,
                         input_data + 3);

  munit_assert_int(parsed_obj.value.dictionary.entries[1].value.type, ==,
                   INTEGER);

  munit_assert_int64(parsed_obj.value.dictionary.entries[1].value.value.integer,
                     ==, 42);

  munit_assert_memory_equal(3, expected_second_key,
                            parsed_obj.value.dictionary.entries[1].key.data);

  munit_assert_size(parsed_obj.value.dictionary.entries[1].key.length, ==, 3);

  munit_assert_ptr_equal(parsed_obj.value.dictionary.entries[1].key.data,
                         input_data + 11);

  free_bencode_object(&parsed_obj);

  return MUNIT_OK;
}
static MunitResult test_bencode_buffer_rejects_missing_closing_e_in_dictionary(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "d3:bari7e3:fooi42e";

  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 0;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, INVALID);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, dummy_count);
  munit_assert_size(parsed_obj.value.dictionary.capacity, ==, dummy_capacity);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, ==, NULL);

  free_bencode_object(&parsed_obj);

  return MUNIT_OK;
}
static MunitResult
test_bencode_buffer_rejects_wrong_closing_syntax_in_dictionary(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "d3:bari7e3:fooi42ex";

  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 0;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, INVALID);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, dummy_count);
  munit_assert_size(parsed_obj.value.dictionary.capacity, ==, dummy_capacity);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, ==, NULL);

  free_bencode_object(&parsed_obj);

  return MUNIT_OK;
}

static MunitResult test_bencode_buffer_parses_dictionary_with_inner_list(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "d3:bari7e3:fool4:testee";

  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;
  const unsigned char expected_first_key[] = "bar";
  const unsigned char expected_second_key[] = "foo";
  const unsigned char expected_first_list_element[] = "test";

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, DICTIONARY);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, 2);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, !=, NULL);
  munit_assert_true(parsed_obj.value.dictionary.capacity >=
                    parsed_obj.value.dictionary.count);

  munit_assert_int(parsed_obj.value.dictionary.entries[0].value.type, ==,
                   INTEGER);
  munit_assert_int64(parsed_obj.value.dictionary.entries[0].value.value.integer,
                     ==, 7);

  munit_assert_memory_equal(3, expected_first_key,
                            parsed_obj.value.dictionary.entries[0].key.data);

  munit_assert_size(parsed_obj.value.dictionary.entries[0].key.length, ==, 3);

  munit_assert_ptr_equal(parsed_obj.value.dictionary.entries[0].key.data,
                         input_data + 3);

  munit_assert_memory_equal(3, expected_second_key,
                            parsed_obj.value.dictionary.entries[1].key.data);
  munit_assert_int(parsed_obj.value.dictionary.entries[1].value.type, ==, LIST);
  munit_assert_size(
      parsed_obj.value.dictionary.entries[1].value.value.list.count, ==, 1);
  munit_assert_size(
      parsed_obj.value.dictionary.entries[1].value.value.list.capacity, ==, 1);

  munit_assert_int(
      parsed_obj.value.dictionary.entries[1].value.value.list.items[0].type, ==,
      BYTE_STRING);
  munit_assert_memory_equal(4, expected_first_list_element,
                            parsed_obj.value.dictionary.entries[1]
                                .value.value.list.items[0]
                                .value.byte_string.data);

  munit_assert_ptr_equal(parsed_obj.value.dictionary.entries[1]
                             .value.value.list.items[0]
                             .value.byte_string.data,
                         input_data + 17);

  free_bencode_object(&parsed_obj);

  return MUNIT_OK;
}

static MunitResult rejects_partially_parsed_dictionary_with_heap_allocation(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "d3:barl4:teste3:fooi42e";

  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 0;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, INVALID);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, dummy_count);
  munit_assert_size(parsed_obj.value.dictionary.capacity, ==, dummy_capacity);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, ==, NULL);

  free_bencode_object(&parsed_obj);

  return MUNIT_OK;
}

static MunitResult test_parse_bencode_buffer_rejects_wrong_order_of_keys(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "d3:fooi42e3:bari7ee";

  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 0;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, INVALID);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, dummy_count);
  munit_assert_size(parsed_obj.value.dictionary.capacity, ==, dummy_capacity);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, ==, NULL);

  return MUNIT_OK;
}
static MunitResult
test_parse_bencode_buffer_rejects_duplicate_keys(const MunitParameter params[],
                                                 void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "d3:fooi1e3:fooi2ee";

  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = 0;

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_false(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, INVALID);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, dummy_count);
  munit_assert_size(parsed_obj.value.dictionary.capacity, ==, dummy_capacity);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, ==, NULL);

  return MUNIT_OK;
}
static MunitResult
test_parse_bencode_buffer_parses_sorted_keys_with_same_prefix(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  parser_state_t parser;
  const unsigned char input_data[] = "d1:ai1e2:aai2ee";
  size_t input_length = sizeof(input_data) - 1;
  size_t expected_parser_position_after_parse = sizeof(input_data) - 1;
  const unsigned char expected_first_key[] = "a";
  const unsigned char expected_second_key[] = "aa";

  bool parser_initialzed =
      bencode_parser_init(&parser, input_data, input_length);

  munit_assert_true(parser_initialzed);

  size_t dummy_count = 99;
  size_t dummy_capacity = 123;
  bencode_object_t parsed_obj = {
      .type = INVALID,
      .value.dictionary = {
          .entries = NULL, .count = dummy_count, .capacity = dummy_capacity}};

  bool parsed = parse_bencode_buffer(&parser, &parsed_obj);

  munit_assert_true(parsed);
  munit_assert_size(parser.position, ==, expected_parser_position_after_parse);
  munit_assert_int(parsed_obj.type, ==, DICTIONARY);
  munit_assert_size(parsed_obj.value.dictionary.count, ==, 2);
  munit_assert_ptr(parsed_obj.value.dictionary.entries, !=, NULL);
  munit_assert_true(parsed_obj.value.dictionary.capacity >=
                    parsed_obj.value.dictionary.count);

  munit_assert_int(parsed_obj.value.dictionary.entries[0].value.type, ==,
                   INTEGER);
  munit_assert_int64(parsed_obj.value.dictionary.entries[0].value.value.integer,
                     ==, 1);

  munit_assert_memory_equal(1, expected_first_key,
                            parsed_obj.value.dictionary.entries[0].key.data);

  munit_assert_size(parsed_obj.value.dictionary.entries[0].key.length, ==, 1);

  munit_assert_ptr_equal(parsed_obj.value.dictionary.entries[0].key.data,
                         input_data + 3);

  munit_assert_int(parsed_obj.value.dictionary.entries[1].value.type, ==,
                   INTEGER);
  munit_assert_int64(parsed_obj.value.dictionary.entries[1].value.value.integer,
                     ==, 2);

  munit_assert_memory_equal(2, expected_second_key,
                            parsed_obj.value.dictionary.entries[1].key.data);

  munit_assert_size(parsed_obj.value.dictionary.entries[1].key.length, ==, 2);

  munit_assert_ptr_equal(parsed_obj.value.dictionary.entries[1].key.data,
                         input_data + 9);

  free_bencode_object(&parsed_obj);

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
    {"/parse/integer/rejects_int64_overflow",
     test_parse_bencode_buffer_rejects_integer_overflow, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/integer/parse_negative_int64",
     test_parse_bencode_buffer_parse_parses_negative_integer, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/integer/returns-false-empty-negative-integer",
     test_parse_bencode_buffer_parse_rejects_empty_negative_integer_syntax,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/integer/reject-negative-zero",
     test_parse_bencode_buffer_rejects_negative_zero, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/integer/accepts_and_parses_INT_64_MIN",
     test_parse_bencode_buffer_accept_INT64_MIN, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/integer/rejects_below_INT_64_MIN",
     test_parse_bencode_buffer_rejects_below_INT64_MIN, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/lists/parse_empty_list",
     test_parse_bencode_buffer_parses_empty_list, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/lists/single_integer_in_list",
     test_parse_bencode_buffer_parses_single_integer_in_list, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/lists/multiple_integer_in_list",
     test_parse_bencode_buffer_parses_multiple_integer_in_list, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/lists/multiple-no-heap-mixed-types-in-list",
     test_parse_bencode_buffer_parses_mixed_elements_in_list, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/lists/parse-inner-lists-with-elements-inside",
     test_parse_bencode_buffer_parses_inner_lists, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/free/frees-inner-elements-correctly",
     test_free_bencode_buffer_parses_frees_and_resets_inner_elements_correctly,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/lists/rejects_partially_parsed_lists",
     test_free_bencode_buffer_reject_partially_parsed_list_and_cleans_ups, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/parses-empty-dictionary",
     test_bencode_buffer_parses_empty_dictionary, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/parses-single-entry-dictionary",
     test_bencode_buffer_parses_dictionary_with_single_element, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/parses-two-entry-dictionary",
     test_bencode_buffer_parses_dictionary_with_two_elements, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/rejects-missing-closing-e-dictionary",
     test_bencode_buffer_rejects_missing_closing_e_in_dictionary, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/rejects-wrong-closing-syntax-dictionary",
     test_bencode_buffer_rejects_wrong_closing_syntax_in_dictionary, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/parses-dictionary-with-inner-list",
     test_bencode_buffer_parses_dictionary_with_inner_list, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/rejects-partially-parsed-with-correct-cleanup",
     rejects_partially_parsed_dictionary_with_heap_allocation, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/reject-wrong-key-order",
     test_parse_bencode_buffer_rejects_wrong_order_of_keys, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/reject-duplicate-keys",
     test_parse_bencode_buffer_rejects_duplicate_keys, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/parse/dictionaries/parses-sorted-keys-same-prefix",
     test_parse_bencode_buffer_parses_sorted_keys_with_same_prefix, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite bencode_parser_suite = {"/bencode_parser", tests, NULL, 1,
                                         MUNIT_SUITE_OPTION_NONE};
