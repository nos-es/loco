#include "torrent_metadata.h"
#include "bencode_types.h"
#include <stddef.h>
#include <string.h>

static const unsigned char info_key_name[] = "info";
static const size_t info_key_name_len = sizeof(info_key_name) - 1;

const bencode_object_t *
torrent_metadata_find_info(const bencode_object_t *root) {
  if (root == NULL) {
    return NULL;
  }
  if (root->type != DICTIONARY) {
    return NULL;
  }

  for (size_t i = 0; i < root->value.dictionary.count; i++) {

    const bencode_dictionary_entry_t *current_entry =
        &root->value.dictionary.entries[i];

    if (current_entry->key.length != info_key_name_len) {
      continue;
    }

    int compare_result = memcmp(current_entry->key.data, info_key_name,
                                current_entry->key.length);
    if (compare_result != 0) {
      continue;
    }

    if (current_entry->value.type != DICTIONARY) {
      continue;
    }

    return &current_entry->value;
  }
  return NULL;
}
