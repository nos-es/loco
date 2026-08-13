#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct BencodeObject bencode_object_t;

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

typedef union BencodeValue {
  int64_t integer;
  bencode_segment_t byte_string;
  bencode_list_t list;
} bencode_value_t;

typedef struct BencodeObject {
  bencode_data_type_t type;
  bencode_value_t value;
} bencode_object_t;
