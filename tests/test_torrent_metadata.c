#include "bencode_types.h"
#include "munit.h"
#include "torrent_metadata.h"
#include <stddef.h>
#include <stdint.h>

static MunitResult
test_torrent_metadata_find_info_finds_info_dict(const MunitParameter params[],
                                                void *user_data) {
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
static MunitTest tests[] = {
    {"/finds_info_dict",
     test_torrent_metadata_find_info_finds_info_dict, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite torrent_metadata_suite = {"/torrent_metadata", tests, NULL, 1,
                                 MUNIT_SUITE_OPTION_NONE};
