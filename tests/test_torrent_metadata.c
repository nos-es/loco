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
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite torrent_metadata_suite = {"/torrent_metadata", tests, NULL, 1,
                                           MUNIT_SUITE_OPTION_NONE};
