#include "bencode_parser.h"
#include <stdlib.h>
bool bencode_parser_init(parser_state_t *parser, const unsigned char *data,
                         size_t length) {
  if (parser == NULL) {
    return false;
  }

  if (data == NULL && length > 0) {
    return false;
  }
  parser->data = data;
  parser->length = length;
  parser->position = 0;

  return true;
}
