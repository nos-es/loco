#include "bencode_parser.h"
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static bool determine_byte_string_length(parser_state_t *parser,
                                         size_t *out_string_length) {

  if (out_string_length == NULL) {
    return false;
  }

  if (parser == NULL) {
    return false;
  }

  size_t length = 0;
  size_t starting_position = parser->position;
  unsigned char current_byte;
  bool consume_result = consume_current_byte(parser, &current_byte);

  if (!consume_result) {
    parser->position = starting_position;
    return false;
  }

  if (!isdigit(current_byte)) {
    parser->position = starting_position;
    return false;
  }

  while (isdigit(current_byte)) {

    size_t digit = current_byte - '0';

    // if true, then size_t overflow.
    if (length > (SIZE_MAX - digit) / 10) {

      parser->position = starting_position;
      return false;
    }

    length = length * 10 + digit;

    bool consume_result = consume_current_byte(parser, &current_byte);

    if (!consume_result) {
      parser->position = starting_position;
      return false;
    }
  }

  if (current_byte == ':') {
    *out_string_length = length;
    return true;
  }

  parser->position = starting_position;

  return false;
}

static bool get_available_byte_count(const parser_state_t *parser,
                                     size_t *out_available_bytes) {

  if (parser == NULL || out_available_bytes == NULL) {
    return false;
  }

  if (parser->position > parser->length) {
    return false;
  }

  *out_available_bytes = parser->length - parser->position;

  return true;
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
  printf("byte obtained result: %d\n", byte_obtained);
  bencode_data_type_t out_byte_datatype = determine_bencode_data_type(out_byte);

  printf("Bencode Datatype: %d\n", out_byte_datatype);

  if (out_byte_datatype == BYTE_STRING) {

    size_t string_len;
    bool string_length_result =
        determine_byte_string_length(parser, &string_len);

    if (!string_length_result) {
      fprintf(stderr, "Something went wrong while determining string length\n");
      return false;
    }
    printf("Determined length is %zu\n", string_len);
  }

  // returning false while testing.
  return false;
}
