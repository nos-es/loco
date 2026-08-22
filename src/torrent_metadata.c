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

bool torrent_metadata_extract_info(const bencode_object_t *root,
                                   torrent_info_t *out_info) {
  if (root == NULL || out_info == NULL) {
    return false;
  }
  if (root->type != DICTIONARY) {
    return false;
  }
  torrent_info_t temp_info = {.name = {.data = NULL, .length = 0},
                              .length = 0,
                              .piece_length = 0,
                              .pieces = {.data = NULL, .length = 0},
                              .info_span = {.start_offset = 0, .length = 0}};

  const bencode_object_t *info_dict = torrent_metadata_find_info(root);
  if (info_dict == NULL) {
    return false;
  }
  const bencode_object_t *name = torrent_metadata_find_name(info_dict);
  const bencode_object_t *length = torrent_metadata_find_length(info_dict);
  const bencode_object_t *piece_length =
      torrent_metadata_find_piece_length(info_dict);
  const bencode_object_t *pieces = torrent_metadata_find_pieces(info_dict);

  if (name == NULL || length == NULL || piece_length == NULL ||
      pieces == NULL) {
    return false;
  }
  temp_info.name = name->value.byte_string;
  temp_info.length = length->value.integer;
  temp_info.piece_length = piece_length->value.integer;
  temp_info.pieces = pieces->value.byte_string;
  temp_info.info_span.start_offset = info_dict->start_offset;
  temp_info.info_span.length = info_dict->encoded_length;

  *out_info = temp_info;

  return true;
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
