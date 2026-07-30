#include "bencode_parser.h"
#include <stdio.h>
#include <stdlib.h>

static bool is_valid_parser(parser_state_t *parser, const unsigned char *data,
                            size_t length) {
  if (parser == NULL) {
    return false;
  }

  if (data == NULL && length > 0) {
    return false;
  }
  return true;
}

bool bencode_parser_init(parser_state_t *parser, const unsigned char *data,
                         size_t length) {
  if (is_valid_parser(parser, data, length) == false) {
    return false;
  }
  parser->data = data;
  parser->length = length;
  parser->position = 0;

  return true;
}

static bool byte_exists_at_position(const parser_state_t *parser) {

  if (parser == NULL) {
    return false;
  }

  if (parser->position < parser->length) {
    return true;
  }

  return false;
}
static bool peek_current_byte(const parser_state_t *parser,
                              unsigned char *out_byte) {
  if (parser == NULL) {
    return false;
  }
  if (out_byte == NULL) {
    return false;
  }
  if (!byte_exists_at_position(parser)) {
    return false;
  }

  if (parser->data == NULL) {
    return false;
  }

  *out_byte = parser->data[parser->position];
  return true;
}
static bool consume_current_byte(parser_state_t *parser,
                                 unsigned char *out_byte) {
  bool byte_obtained = peek_current_byte(parser, out_byte);

  if (!byte_obtained) {
    return false;
  }

  parser->position++;

  return true;
}
static bencode_data_type_t determine_bencode_data_type(unsigned char byte) {

  if (byte >= '0' && byte <= '9') {
    return BYTE_STRING;
  }

  switch (byte) {
  case 'i':
    return INTEGER;
  case 'l':
    return LIST;
  case 'd':
    return DICTIONARY;
  default:
    return INVALID;
  }
}

bool parse_bencode_buffer(parser_state_t *parser) {

  unsigned char out_byte;

  bool byte_obtained = peek_current_byte(parser, &out_byte);

  if (!byte_obtained) {
    printf("Byte could not be obtained");
    return false;
  }
  printf("Obtained byte: %c\n", out_byte);
  printf("Parser position: %zu\n", parser->position);
  printf("result: %d\n", byte_obtained);

  printf("Bencode Datatype: %d\n", determine_bencode_data_type(out_byte));

  // returning false while testing.
  return false;
}
