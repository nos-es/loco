#include "torrent_metadata.h"
#include "bencode_types.h"
#include <stddef.h>
#include <string.h>

static const bencode_object_t *
find_entry_in_dictionary(const bencode_object_t *root,
                         const unsigned char *key_name, size_t key_length,
                         bencode_data_type_t target_type) {
  if (root == NULL) {
    return NULL;
  }
  if (root->type != DICTIONARY) {
    return NULL;
  }
  for (size_t i = 0; i < root->value.dictionary.count; i++) {

    const bencode_dictionary_entry_t *current_entry =
        &root->value.dictionary.entries[i];

    if (current_entry->key.length != key_length) {
      continue;
    }

    int compare_result =
        memcmp(current_entry->key.data, key_name, current_entry->key.length);
    if (compare_result != 0) {
      continue;
    }

    if (current_entry->value.type != target_type) {
      continue;
    }

    return &current_entry->value;
  }
  return NULL;
}
const bencode_object_t *
torrent_metadata_find_info(const bencode_object_t *root) {

  const unsigned char info_key_name[] = "info";
  const size_t key_len = sizeof(info_key_name) - 1;

  return find_entry_in_dictionary(root, info_key_name, key_len, DICTIONARY);
}

const bencode_object_t *
torrent_metadata_find_name(const bencode_object_t *info) {

  static const unsigned char name_key_name[] = "name";
  const size_t key_len = sizeof(name_key_name) - 1;

  return find_entry_in_dictionary(info, name_key_name, key_len, BYTE_STRING);
}

const bencode_object_t *
torrent_metadata_find_length(const bencode_object_t *info) {

  static const unsigned char length_key_name[] = "length";
  const size_t key_len = sizeof(length_key_name) - 1;
  const bencode_object_t *bencode_obj =
      find_entry_in_dictionary(info, length_key_name, key_len, INTEGER);

  if (bencode_obj == NULL) {
    return NULL;
  }
  if (bencode_obj->value.integer < 0) {
    return NULL;
  }

  return bencode_obj;
}

const bencode_object_t *
torrent_metadata_find_piece_length(const bencode_object_t *info) {

  static const unsigned char piece_length_key_name[] = "piece length";
  const size_t key_len = sizeof(piece_length_key_name) - 1;
  const bencode_object_t *bencode_obj =
      find_entry_in_dictionary(info, piece_length_key_name, key_len, INTEGER);

  if (bencode_obj == NULL) {
    return NULL;
  }
  if (bencode_obj->value.integer <= 0) {
    return NULL;
  }

  return bencode_obj;
}

const bencode_object_t *
torrent_metadata_find_pieces(const bencode_object_t *info) {

  static const unsigned char pieces_key_name[] = "pieces";
  const size_t key_len = sizeof(pieces_key_name) - 1;
  const bencode_object_t *bencode_obj =
      find_entry_in_dictionary(info, pieces_key_name, key_len, BYTE_STRING);

  if (bencode_obj == NULL) {
    return NULL;
  }

  if (bencode_obj->value.byte_string.length % 20 != 0) {
    return NULL;
  }

  return bencode_obj;
}
