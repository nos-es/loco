#include "bencode_types.h"
#include "munit.h"
#include "torrent_metadata.h"
#include <stddef.h>
#include <stdint.h>

static MunitResult test_torrent_metadata_find_info_finds_valid_info_dict(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char info_key[] = "info";
  bencode_dictionary_entry_t info_entry = {.key.data = info_key,
                                           .key.length = sizeof(info_key) - 1,
                                           .value.type = DICTIONARY,
                                           .value.value.dictionary.entries =
                                               NULL,
                                           .value.value.dictionary.capacity = 0,
                                           .value.value.dictionary.count = 0};

  bencode_dictionary_entry_t info_entry_slot[1];
  bencode_object_t root_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = info_entry_slot,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  root_dict.value.dictionary.entries[0] = info_entry;

  const bencode_object_t *result = torrent_metadata_find_info(&root_dict);
  munit_assert_not_null(result);
  munit_assert_int(result->type, ==, DICTIONARY);
  munit_assert_ptr_equal(result, &root_dict.value.dictionary.entries[0].value);

  return MUNIT_OK;
}
static MunitResult
test_torrent_metadata_find_info_rejects_wrong_info_value_type(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char info_key[] = "info";
  bencode_dictionary_entry_t info_entry = {.key.data = info_key,
                                           .key.length = sizeof(info_key) - 1,
                                           .value.type = INTEGER,
                                           .value.value.integer = 42};

  bencode_dictionary_entry_t info_entry_slot[1];
  const bencode_object_t root_dict = {.type = DICTIONARY,
                                      .value.dictionary.entries =
                                          info_entry_slot,
                                      .value.dictionary.count = 1,
                                      .value.dictionary.capacity = 1};

  root_dict.value.dictionary.entries[0] = info_entry;

  const bencode_object_t *result = torrent_metadata_find_info(&root_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}
static MunitResult
test_torrent_metadata_find_info_finds_info_entry_in_dictionary_with_multiple_keys(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char info_key[] = "info";
  bencode_dictionary_entry_t info_entry = {.key.data = info_key,
                                           .key.length = sizeof(info_key) - 1,
                                           .value.type = DICTIONARY,
                                           .value.value.dictionary.entries =
                                               NULL,
                                           .value.value.dictionary.capacity = 0,
                                           .value.value.dictionary.count = 0};

  const unsigned char first_dummy_key[] = "first_dummy";
  bencode_dictionary_entry_t first_dummy_entry = {
      .key.data = first_dummy_key,
      .key.length = sizeof(first_dummy_key) - 1,
      .value.type = INTEGER,
      .value.value.integer = 42};

  const unsigned char second_dummy_key[] = "second_dummy";
  bencode_dictionary_entry_t second_dummy_entry = {
      .key.data = second_dummy_key,
      .key.length = sizeof(second_dummy_key) - 1,
      .value.type = DICTIONARY,
      .value.value.dictionary.entries = NULL,
      .value.value.dictionary.capacity = 0,
      .value.value.dictionary.count = 0};

  bencode_dictionary_entry_t entries[3];
  bencode_object_t root_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 3,
                                .value.dictionary.capacity = 3};

  root_dict.value.dictionary.entries[0] = first_dummy_entry;
  root_dict.value.dictionary.entries[1] = info_entry;
  root_dict.value.dictionary.entries[2] = second_dummy_entry;

  const bencode_object_t *result = torrent_metadata_find_info(&root_dict);
  munit_assert_not_null(result);
  munit_assert_int(result->type, ==, DICTIONARY);
  munit_assert_ptr_equal(result, &root_dict.value.dictionary.entries[1].value);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_info_rejects_null_root(const MunitParameter params[],
                                                  void *user_data) {
  (void)params;
  (void)user_data;

  const bencode_object_t *result = torrent_metadata_find_info(NULL);
  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult test_torrent_metadata_find_info_rejects_wrong_root_type(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  const bencode_object_t root_dict = {.type = INTEGER, .value.integer = 42};

  const bencode_object_t *result = torrent_metadata_find_info(&root_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}
static MunitResult
test_torrent_metadata_find_info_rejects_dict_with_missing_info_key(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  const unsigned char first_dummy_key[] = "first_dummy";
  bencode_dictionary_entry_t first_dummy_entry = {
      .key.data = first_dummy_key,
      .key.length = sizeof(first_dummy_key) - 1,
      .value.type = INTEGER,
      .value.value.integer = 42};

  const unsigned char second_dummy_key[] = "second_dummy";
  bencode_dictionary_entry_t second_dummy_entry = {
      .key.data = second_dummy_key,
      .key.length = sizeof(second_dummy_key) - 1,
      .value.type = DICTIONARY,
      .value.value.dictionary.entries = NULL,
      .value.value.dictionary.capacity = 0,
      .value.value.dictionary.count = 0};

  bencode_dictionary_entry_t entries[2];
  bencode_object_t root_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 2,
                                .value.dictionary.capacity = 2};

  root_dict.value.dictionary.entries[0] = first_dummy_entry;
  root_dict.value.dictionary.entries[1] = second_dummy_entry;

  const bencode_object_t *result = torrent_metadata_find_info(&root_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_name_rejects_wrong_name_value_type(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char name_key[] = "name";
  bencode_dictionary_entry_t name_entry = {.key.data = name_key,
                                           .key.length = sizeof(name_key) - 1,
                                           .value.type = INTEGER,
                                           .value.value.integer = 42};

  bencode_dictionary_entry_t name_entry_slot[1];
  const bencode_object_t info_dict = {.type = DICTIONARY,
                                      .value.dictionary.entries =
                                          name_entry_slot,
                                      .value.dictionary.count = 1,
                                      .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = name_entry;

  const bencode_object_t *result = torrent_metadata_find_name(&info_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult test_torrent_metadata_find_name_finds_valid_name_key(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char name_key[] = "name";
  const unsigned char byte_string[] = "test";
  const size_t byte_string_len = sizeof(byte_string) - 1;

  bencode_dictionary_entry_t name_entry = {
      .key.data = name_key,
      .key.length = sizeof(name_key) - 1,
      .value.type = BYTE_STRING,
      .value.value.byte_string.data = byte_string,
      .value.value.byte_string.length = byte_string_len};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = name_entry;

  const bencode_object_t *result = torrent_metadata_find_name(&info_dict);
  munit_assert_not_null(result);
  munit_assert_ptr_equal(result, &info_dict.value.dictionary.entries[0].value);
  munit_assert_int(result->type, ==, BYTE_STRING);
  munit_assert_memory_equal(byte_string_len, result->value.byte_string.data,
                            byte_string);
  munit_assert_size(result->value.byte_string.length, ==, byte_string_len);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_name_rejects_dict_with_missing_name_key(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  const unsigned char first_dummy_key[] = "first_dummy";
  bencode_dictionary_entry_t first_dummy_entry = {
      .key.data = first_dummy_key,
      .key.length = sizeof(first_dummy_key) - 1,
      .value.type = INTEGER,
      .value.value.integer = 42};

  const unsigned char second_dummy_key[] = "second_dummy";
  bencode_dictionary_entry_t second_dummy_entry = {
      .key.data = second_dummy_key,
      .key.length = sizeof(second_dummy_key) - 1,
      .value.type = DICTIONARY,
      .value.value.dictionary.entries = NULL,
      .value.value.dictionary.capacity = 0,
      .value.value.dictionary.count = 0};

  bencode_dictionary_entry_t entries[2];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 2,
                                .value.dictionary.capacity = 2};

  info_dict.value.dictionary.entries[0] = first_dummy_entry;
  info_dict.value.dictionary.entries[1] = second_dummy_entry;

  const bencode_object_t *result = torrent_metadata_find_name(&info_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult test_torrent_metadata_find_length_finds_valid_length_key(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char length_key[] = "length";
  const size_t key_len = sizeof(length_key) - 1;
  const int64_t integer_value = 42;

  bencode_dictionary_entry_t length_entry = {.key.data = length_key,
                                             .key.length = key_len,
                                             .value.type = INTEGER,
                                             .value.value.integer =
                                                 integer_value};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = length_entry;

  const bencode_object_t *result = torrent_metadata_find_length(&info_dict);
  munit_assert_not_null(result);
  munit_assert_ptr_equal(result, &info_dict.value.dictionary.entries[0].value);
  munit_assert_int(result->type, ==, INTEGER);
  munit_assert_int64(result->value.integer, ==, integer_value);

  return MUNIT_OK;
}

static MunitResult test_torrent_metadata_find_length_rejects_negative_length(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char length_key[] = "length";
  const size_t key_len = sizeof(length_key) - 1;
  const int64_t integer_value = -42;

  bencode_dictionary_entry_t length_entry = {.key.data = length_key,
                                             .key.length = key_len,
                                             .value.type = INTEGER,
                                             .value.value.integer =
                                                 integer_value};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = length_entry;

  const bencode_object_t *result = torrent_metadata_find_length(&info_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_length_rejects_dict_with_missing_length_key(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  const unsigned char first_dummy_key[] = "first_dummy";
  bencode_dictionary_entry_t first_dummy_entry = {
      .key.data = first_dummy_key,
      .key.length = sizeof(first_dummy_key) - 1,
      .value.type = INTEGER,
      .value.value.integer = 42};

  const unsigned char second_dummy_key[] = "second_dummy";
  bencode_dictionary_entry_t second_dummy_entry = {
      .key.data = second_dummy_key,
      .key.length = sizeof(second_dummy_key) - 1,
      .value.type = DICTIONARY,
      .value.value.dictionary.entries = NULL,
      .value.value.dictionary.capacity = 0,
      .value.value.dictionary.count = 0};

  bencode_dictionary_entry_t entries[2];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 2,
                                .value.dictionary.capacity = 2};

  info_dict.value.dictionary.entries[0] = first_dummy_entry;
  info_dict.value.dictionary.entries[1] = second_dummy_entry;

  const bencode_object_t *result = torrent_metadata_find_length(&info_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_length_rejects_wrong_length_value_type(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char length_key[] = "length";
  const size_t key_len = sizeof(length_key) - 1;
  const unsigned char key_value[] = "42";
  const unsigned char value_len = sizeof(key_value) - 1;
  const bencode_data_type_t wrong_type = BYTE_STRING;

  bencode_dictionary_entry_t length_entry = {
      .key.data = length_key,
      .key.length = key_len,
      .value.type = wrong_type,
      .value.value.byte_string.data = key_value,
      .value.value.byte_string.length = value_len};

  bencode_dictionary_entry_t entries[1];
  const bencode_object_t info_dict = {.type = DICTIONARY,
                                      .value.dictionary.entries = entries,
                                      .value.dictionary.count = 1,
                                      .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = length_entry;

  const bencode_object_t *result = torrent_metadata_find_length(&info_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}
static MunitResult test_torrent_metadata_find_length_accepts_zero_length(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char length_key[] = "length";
  const size_t key_len = sizeof(length_key) - 1;
  const int64_t integer_value = 0;

  bencode_dictionary_entry_t length_entry = {.key.data = length_key,
                                             .key.length = key_len,
                                             .value.type = INTEGER,
                                             .value.value.integer =
                                                 integer_value};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = length_entry;

  const bencode_object_t *result = torrent_metadata_find_length(&info_dict);
  munit_assert_not_null(result);
  munit_assert_ptr_equal(result, &info_dict.value.dictionary.entries[0].value);
  munit_assert_int(result->type, ==, INTEGER);
  munit_assert_int64(result->value.integer, ==, integer_value);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_piece_length_returns_valid_piece_length(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char piece_length_key[] = "piece length";
  const size_t key_len = sizeof(piece_length_key) - 1;
  const int64_t integer_value = 42;

  bencode_dictionary_entry_t piece_length_entry = {.key.data = piece_length_key,
                                                   .key.length = key_len,
                                                   .value.type = INTEGER,
                                                   .value.value.integer =
                                                       integer_value};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = piece_length_entry;

  const bencode_object_t *result =
      torrent_metadata_find_piece_length(&info_dict);

  munit_assert_not_null(result);
  munit_assert_ptr_equal(result, &info_dict.value.dictionary.entries[0].value);
  munit_assert_int(result->type, ==, INTEGER);
  munit_assert_int64(result->value.integer, ==, integer_value);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_piece_length_rejects_negative_length(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char piece_length_key[] = "piece length";
  const size_t key_len = sizeof(piece_length_key) - 1;
  const int64_t integer_value = -42;

  bencode_dictionary_entry_t piece_length_entry = {.key.data = piece_length_key,
                                                   .key.length = key_len,
                                                   .value.type = INTEGER,
                                                   .value.value.integer =
                                                       integer_value};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = piece_length_entry;

  const bencode_object_t *result =
      torrent_metadata_find_piece_length(&info_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_piece_length_rejects_dict_with_missing_piece_length_key(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  const unsigned char first_dummy_key[] = "first_dummy";
  bencode_dictionary_entry_t first_dummy_entry = {
      .key.data = first_dummy_key,
      .key.length = sizeof(first_dummy_key) - 1,
      .value.type = INTEGER,
      .value.value.integer = 42};

  const unsigned char second_dummy_key[] = "second_dummy";
  bencode_dictionary_entry_t second_dummy_entry = {
      .key.data = second_dummy_key,
      .key.length = sizeof(second_dummy_key) - 1,
      .value.type = DICTIONARY,
      .value.value.dictionary.entries = NULL,
      .value.value.dictionary.capacity = 0,
      .value.value.dictionary.count = 0};

  bencode_dictionary_entry_t entries[2];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 2,
                                .value.dictionary.capacity = 2};

  info_dict.value.dictionary.entries[0] = first_dummy_entry;
  info_dict.value.dictionary.entries[1] = second_dummy_entry;

  const bencode_object_t *result =
      torrent_metadata_find_piece_length(&info_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_piece_length_rejects_wrong_value_type(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char piece_length_key[] = "piece length";
  const size_t key_len = sizeof(piece_length_key) - 1;
  const unsigned char key_value[] = "42";
  const size_t value_len = sizeof(key_value) - 1;
  const bencode_data_type_t wrong_type = BYTE_STRING;

  bencode_dictionary_entry_t piece_length_entry = {
      .key.data = piece_length_key,
      .key.length = key_len,
      .value.type = wrong_type,
      .value.value.byte_string.data = key_value,
      .value.value.byte_string.length = value_len};

  bencode_dictionary_entry_t entries[1];
  const bencode_object_t info_dict = {.type = DICTIONARY,
                                      .value.dictionary.entries = entries,
                                      .value.dictionary.count = 1,
                                      .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = piece_length_entry;

  const bencode_object_t *result =
      torrent_metadata_find_piece_length(&info_dict);
  munit_assert_null(result);

  return MUNIT_OK;
}
static MunitResult test_torrent_metadata_find_piece_length_rejects_zero_length(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char piece_length_key[] = "piece length";
  const size_t key_len = sizeof(piece_length_key) - 1;
  const int64_t integer_value = 0;

  bencode_dictionary_entry_t piece_length_entry = {.key.data = piece_length_key,
                                                   .key.length = key_len,
                                                   .value.type = INTEGER,
                                                   .value.value.integer =
                                                       integer_value};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = piece_length_entry;

  const bencode_object_t *result =
      torrent_metadata_find_piece_length(&info_dict);

  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult test_torrent_metadata_find_pieces_returns_valid_pieces_value(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char pieces_key[] = "pieces";
  const unsigned char byte_string[] =
      "abcdefghijklmnopqrstabcdefghijklmnopqrst";
  const size_t byte_string_len = sizeof(byte_string) - 1;

  bencode_dictionary_entry_t pieces_entry = {
      .key.data = pieces_key,
      .key.length = sizeof(pieces_key) - 1,
      .value.type = BYTE_STRING,
      .value.value.byte_string.data = byte_string,
      .value.value.byte_string.length = byte_string_len};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = pieces_entry;

  const bencode_object_t *result = torrent_metadata_find_pieces(&info_dict);

  munit_assert_not_null(result);
  munit_assert_ptr_equal(result, &info_dict.value.dictionary.entries[0].value);
  munit_assert_int(result->type, ==, BYTE_STRING);
  munit_assert_memory_equal(byte_string_len, result->value.byte_string.data,
                            byte_string);
  munit_assert_size(result->value.byte_string.length, ==, byte_string_len);

  return MUNIT_OK;
}

static MunitResult test_torrent_metadata_find_pieces_rejects_missing_pieces_key(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  const unsigned char first_dummy_key[] = "first_dummy";
  bencode_dictionary_entry_t first_dummy_entry = {
      .key.data = first_dummy_key,
      .key.length = sizeof(first_dummy_key) - 1,
      .value.type = INTEGER,
      .value.value.integer = 42};

  const unsigned char second_dummy_key[] = "second_dummy";
  bencode_dictionary_entry_t second_dummy_entry = {
      .key.data = second_dummy_key,
      .key.length = sizeof(second_dummy_key) - 1,
      .value.type = DICTIONARY,
      .value.value.dictionary.entries = NULL,
      .value.value.dictionary.capacity = 0,
      .value.value.dictionary.count = 0};

  bencode_dictionary_entry_t entries[2];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 2,
                                .value.dictionary.capacity = 2};

  info_dict.value.dictionary.entries[0] = first_dummy_entry;
  info_dict.value.dictionary.entries[1] = second_dummy_entry;

  const bencode_object_t *result = torrent_metadata_find_pieces(&info_dict);

  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult test_torrent_metadata_find_pieces_rejects_wrong_value_type(
    const MunitParameter params[], void *user_data) {

  (void)params;
  (void)user_data;

  const unsigned char pieces_key[] = "pieces";
  const int64_t integer_value = 42;

  bencode_dictionary_entry_t pieces_entry = {
      .key.data = pieces_key,
      .key.length = sizeof(pieces_key) - 1,
      .value.type = INTEGER,
      .value.value.integer = integer_value};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = pieces_entry;

  const bencode_object_t *result = torrent_metadata_find_pieces(&info_dict);

  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_find_pieces_rejects_length_not_divisible_by_20(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;
  const unsigned char pieces_key[] = "pieces";
  const unsigned char byte_string[] = "abcdefghijklmnopqrstabcdefghijklmnopq";
  const size_t byte_string_len = sizeof(byte_string) - 1;

  bencode_dictionary_entry_t pieces_entry = {
      .key.data = pieces_key,
      .key.length = sizeof(pieces_key) - 1,
      .value.type = BYTE_STRING,
      .value.value.byte_string.data = byte_string,
      .value.value.byte_string.length = byte_string_len};

  bencode_dictionary_entry_t entries[1];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  info_dict.value.dictionary.entries[0] = pieces_entry;

  const bencode_object_t *result = torrent_metadata_find_pieces(&info_dict);

  munit_assert_null(result);

  return MUNIT_OK;
}

static MunitResult
test_torrent_metadata_extract_info_extracts_valid_info_correctly(
    const MunitParameter params[], void *user_data) {
  (void)params;
  (void)user_data;

  const unsigned char name_key[] = "name";
  const unsigned char name_byte_string[] = "test";
  const size_t name_byte_string_len = sizeof(name_byte_string) - 1;

  bencode_dictionary_entry_t name_entry = {
      .key.data = name_key,
      .key.length = sizeof(name_key) - 1,
      .value.type = BYTE_STRING,
      .value.value.byte_string.data = name_byte_string,
      .value.value.byte_string.length = name_byte_string_len};

  const unsigned char length_key[] = "length";
  const size_t length_key_len = sizeof(length_key) - 1;
  const int64_t length_integer_value = 80;

  bencode_dictionary_entry_t length_entry = {.key.data = length_key,
                                             .key.length = length_key_len,
                                             .value.type = INTEGER,
                                             .value.value.integer =
                                                 length_integer_value};

  const unsigned char piece_length_key[] = "piece length";
  const size_t piece_length_key_len = sizeof(piece_length_key) - 1;
  const int64_t piece_length_integer_value = 40;

  bencode_dictionary_entry_t piece_length_entry = {
      .key.data = piece_length_key,
      .key.length = piece_length_key_len,
      .value.type = INTEGER,
      .value.value.integer = piece_length_integer_value};

  const unsigned char pieces_key[] = "pieces";
  const unsigned char pieces_byte_string[] =
      "abcdefghijklmnopqrstabcdefghijklmnopqrst";
  const size_t pieces_byte_string_len = sizeof(pieces_byte_string) - 1;

  bencode_dictionary_entry_t pieces_entry = {
      .key.data = pieces_key,
      .key.length = sizeof(pieces_key) - 1,
      .value.type = BYTE_STRING,
      .value.value.byte_string.data = pieces_byte_string,
      .value.value.byte_string.length = pieces_byte_string_len};

  bencode_dictionary_entry_t entries[4];
  bencode_object_t info_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = entries,
                                .value.dictionary.count = 4,
                                .value.dictionary.capacity = 4};

  info_dict.value.dictionary.entries[0] = length_entry;
  info_dict.value.dictionary.entries[1] = name_entry;
  info_dict.value.dictionary.entries[2] = piece_length_entry;
  info_dict.value.dictionary.entries[3] = pieces_entry;

  const unsigned char info_key[] = "info";

  bencode_dictionary_entry_t info_entry = {
      .key.data = info_key,
      .key.length = sizeof(info_key) - 1,
  };
  info_entry.value = info_dict;
  bencode_dictionary_entry_t info_entry_slot[1];
  bencode_object_t root_dict = {.type = DICTIONARY,
                                .value.dictionary.entries = info_entry_slot,
                                .value.dictionary.count = 1,
                                .value.dictionary.capacity = 1};

  root_dict.value.dictionary.entries[0] = info_entry;

  torrent_info_t test_info = {.name = {.data = NULL, .length = 0},
                              .length = 0,
                              .piece_length = 0,
                              .pieces = {.data = NULL, .length = 0}};

  bool extracted = torrent_metadata_extract_info(&root_dict, &test_info);

  munit_assert_true(extracted);

  munit_assert_int64(test_info.length, ==, length_integer_value);
  munit_assert_int64(test_info.piece_length, ==, piece_length_integer_value);

  munit_assert_memory_equal(name_byte_string_len, test_info.name.data,
                            name_byte_string);

  munit_assert_ptr_equal(test_info.name.data,
                         root_dict.value.dictionary.entries[0]
                             .value.value.dictionary.entries[1]
                             .value.value.byte_string.data);

  munit_assert_size(test_info.name.length, ==, name_byte_string_len);

  munit_assert_memory_equal(pieces_byte_string_len, test_info.pieces.data,
                            pieces_byte_string);

  munit_assert_ptr_equal(test_info.pieces.data,
                         root_dict.value.dictionary.entries[0]
                             .value.value.dictionary.entries[3]
                             .value.value.byte_string.data);
  munit_assert_size(test_info.pieces.length, ==, pieces_byte_string_len);

  return MUNIT_OK;
}
static MunitTest tests[] = {
    {"/find_info/returns-valid-info-dictionary",
     test_torrent_metadata_find_info_finds_valid_info_dict, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_info/rejects-wrong-info-value-type",
     test_torrent_metadata_find_info_rejects_wrong_info_value_type, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_info/finds-info-entry-in-dictionary-with-multiple-keys",
     test_torrent_metadata_find_info_finds_info_entry_in_dictionary_with_multiple_keys,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_info/rejects-null-root",
     test_torrent_metadata_find_info_rejects_null_root, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_info/rejects-wrong-root-type",
     test_torrent_metadata_find_info_rejects_wrong_root_type, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_info/rejects-dictionary-with-missing-info-key",
     test_torrent_metadata_find_info_rejects_dict_with_missing_info_key, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_name/reject-wrong-name-value-type",
     test_torrent_metadata_find_name_rejects_wrong_name_value_type, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_name/returns-valid-name-byte-string",
     test_torrent_metadata_find_name_finds_valid_name_key, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_name/rejects-dictionary-with-missing-name-key",
     test_torrent_metadata_find_name_rejects_dict_with_missing_name_key, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_length/returns-valid-length",
     test_torrent_metadata_find_length_finds_valid_length_key, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_length/rejects-negative-length",
     test_torrent_metadata_find_length_rejects_negative_length, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_length/rejects-missing-length-key",
     test_torrent_metadata_find_length_rejects_dict_with_missing_length_key,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_length/reject-wrong-length-value-type",
     test_torrent_metadata_find_length_rejects_wrong_length_value_type, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_length/accepts-zero-length",
     test_torrent_metadata_find_length_accepts_zero_length, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_piece_length/returns-valid-piece-length",
     test_torrent_metadata_find_piece_length_returns_valid_piece_length, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_piece_length/rejects-negative-piece-length",
     test_torrent_metadata_find_piece_length_rejects_negative_length, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_piece_length/rejects-missing-piece-length-key",
     test_torrent_metadata_find_piece_length_rejects_dict_with_missing_piece_length_key,
     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_piece_length/reject-wrong-piece_length-value-type",
     test_torrent_metadata_find_piece_length_rejects_wrong_value_type, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_piece_length/rejects-zero-length",
     test_torrent_metadata_find_piece_length_rejects_zero_length, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_pieces/returns-valid-pieces",
     test_torrent_metadata_find_pieces_returns_valid_pieces_value, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_pieces/rejects-missing-pieces-key",
     test_torrent_metadata_find_pieces_rejects_missing_pieces_key, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_pieces/rejects-wrong-pieces-value-type",
     test_torrent_metadata_find_pieces_rejects_wrong_value_type, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_pieces/rejects-byte-length-not-divisble-by-20",
     test_torrent_metadata_find_pieces_rejects_length_not_divisible_by_20, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/extract_info/extract-valid-info",
     test_torrent_metadata_extract_info_extracts_valid_info_correctly, NULL,
     NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite torrent_metadata_suite = {"/torrent_metadata", tests, NULL, 1,
                                           MUNIT_SUITE_OPTION_NONE};
