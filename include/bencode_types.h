#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct BencodeObject bencode_object_t;
typedef struct BencodeDictionaryEntry bencode_dictionary_entry_t;

typedef struct BencodeSegment {
  const unsigned char *data;
  size_t length;
} bencode_segment_t;

typedef struct BencodeList {
  bencode_object_t *items;
  size_t count;
  size_t capacity;

} bencode_list_t;

typedef enum BencodeDataType {
  INTEGER,
  BYTE_STRING,
  LIST,
  DICTIONARY,
  INVALID
} bencode_data_type_t;

typedef struct BencodeDictionary {
  bencode_dictionary_entry_t *entries;
  size_t capacity;
  size_t count;
} bencode_dictionary_t;

typedef union BencodeValue {
  int64_t integer;
  bencode_segment_t byte_string;
  bencode_list_t list;
  bencode_dictionary_t dictionary;
} bencode_value_t;

typedef struct BencodeObject {
  bencode_data_type_t type;
  bencode_value_t value;
  size_t start_offset;
  size_t encoded_length;
} bencode_object_t;

typedef struct BencodeDictionaryEntry {
  bencode_segment_t key;
  bencode_object_t value;
} bencode_dictionary_entry_t;
