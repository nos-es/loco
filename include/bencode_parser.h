#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct ParserState {
  const unsigned char *data;
  size_t length;
  size_t position;
} parser_state_t;

typedef enum BencodeDataType {
  INTEGER,
  BYTE_STRING,
  LIST,
  DICTIONARY,
  INVALID
} bencode_data_type_t;

bool bencode_parser_init(parser_state_t *parser, const unsigned char *data,
                         size_t length);

bool parse_bencode_buffer(parser_state_t *parser);
