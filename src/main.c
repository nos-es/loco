#include "bencode_parser.h"
#include "cli.h"
#include "file_reader.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

  const char *torrent_filepath = NULL;

  if (!cli_arguments_valid(argc, argv, &torrent_filepath)) {
    fprintf(stderr, "Usage: %s <torrent-file>\n", argv[0]);
    return 1;
  }

  byte_buffer_t buffer = {.data = NULL, .length = 0};

  bool result = read_byte_buffer_from_file(torrent_filepath, &buffer);
  if (!result) {
    fprintf(stderr, "Failed to read file.\n");
    free_buffer(&buffer);
    return 1;
  }
  parser_state_t parser;
  bool parse_result = bencode_parser_init(&parser, buffer.data, buffer.length);

  if (!parse_result) {
    free_buffer(&buffer);
    return 1;
  }

  printf("Parse result: %d\n", parse_result);

  parse_bencode_buffer(&parser);

  // free buffer when program ends.
  free_buffer(&buffer);

  return 0;
}
