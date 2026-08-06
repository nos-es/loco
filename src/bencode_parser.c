#include "bencode_parser.h"
#include "bencode_types.h"
#include <ctype.h>
#include <inttypes.h>
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

static bool
bytes_exist_from_current_parser_position(const parser_state_t *parser,
                                         size_t needed_byte_length) {
  if (parser == NULL) {
    return false;
  }

  size_t available_bytes;

  bool byte_count_result = get_available_byte_count(parser, &available_bytes);
  if (!byte_count_result) {
    return false;
  }

  return needed_byte_length <= available_bytes;
}

static bool read_bencode_string(parser_state_t *parser, size_t string_len,
                                bencode_segment_t *out_segment) {
  if (parser == NULL || out_segment == NULL) {
    return false;
  }

  out_segment->data = &parser->data[parser->position];
  out_segment->length = string_len;
  parser->position += string_len;

  return true;
}

static bool parse_bencode_string(parser_state_t *parser,
                                 bencode_segment_t *out_segment) {

  if (parser == NULL || out_segment == NULL) {
    return false;
  }
  size_t string_len;
  size_t start_position = parser->position;

  // returns 4 from for example 4:spam
  bool string_length_determined =
      determine_byte_string_length(parser, &string_len);

  if (!string_length_determined) {
    parser->position = start_position;
    fprintf(stderr, "Something went wrong while determining string length\n");
    return false;
  }

  bool bytes_exist =
      bytes_exist_from_current_parser_position(parser, string_len);
  if (!bytes_exist) {
    parser->position = start_position;
    return false;
  }

  bool bencode_string_read =
      read_bencode_string(parser, string_len, out_segment);

  if (!bencode_string_read) {
    parser->position = start_position;
    return false;
  }

  return true;
}

static bool parse_bencode_integer(parser_state_t *parser,
                                  int64_t *out_integer) {
  if (parser == NULL || out_integer == NULL) {
    return false;
  }

  size_t parser_start_position = parser->position;
  unsigned char current_byte;
  bool first_byte_consumed = consume_current_byte(parser, &current_byte);

  if (first_byte_consumed == false || current_byte != 'i') {
    parser->position = parser_start_position;
    return false;
  }

  bool second_byte_consumed = consume_current_byte(parser, &current_byte);

  if (!second_byte_consumed) {
    parser->position = parser_start_position;
    return false;
  }

  // handles empty integer syntax.(ie)
  if (current_byte == 'e') {
    parser->position = parser_start_position;
    return false;
  }

  // todo: negative sign

  int64_t integer_value = 0;

  while (isdigit(current_byte)) {

    int64_t digit = (int64_t)(current_byte - '0');

    // TODO: check int64_t overflow.

    integer_value = integer_value * 10 + digit;

    bool consume_result = consume_current_byte(parser, &current_byte);

    if (!consume_result) {
      parser->position = parser_start_position;
      return false;
    }

    // handles leading zero
    if (integer_value == 0 && isdigit(current_byte)) {
      parser->position = parser_start_position;
      return false;
    }
  }

  if (current_byte == 'e') {
    *out_integer = integer_value;
    return true;
  }

  parser->position = parser_start_position;
  return false;
}

bool parse_bencode_buffer(parser_state_t *parser,
                          bencode_object_t *out_object) {

  if (parser == NULL || out_object == NULL) {
    return false;
  }

  unsigned char out_byte;
  size_t parser_start_position = parser->position;

  bool byte_obtained = peek_current_byte(parser, &out_byte);

  if (!byte_obtained) {
    printf("Byte could not be obtained\n");
    return false;
  }

  bencode_data_type_t out_byte_datatype = determine_bencode_data_type(out_byte);

  if (out_byte_datatype == BYTE_STRING) {
    bencode_object_t temp_obj = {
        .type = BYTE_STRING, .value.byte_string = {.data = NULL, .length = 0}};

    bool bencode_string_parsed =
        parse_bencode_string(parser, &temp_obj.value.byte_string);

    if (!bencode_string_parsed) {
      parser->position = parser_start_position;
      return false;
    }
    *out_object = temp_obj;
    return true;
  }

  if (out_byte_datatype == INTEGER) {
    bencode_object_t temp_obj = {
        .type = INTEGER, .value.byte_string = {.data = NULL, .length = 0}};

    bool bencode_integer_parsed =
        parse_bencode_integer(parser, &temp_obj.value.integer);

    if (!bencode_integer_parsed) {
      parser->position = parser_start_position;
      return false;
    }
    *out_object = temp_obj;
    return true;
  }

  return false;
}
