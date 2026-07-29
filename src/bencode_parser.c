#include "bencode_parser.h"
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
