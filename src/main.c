#include "bencode_parser.h"
#include "bencode_types.h"
#include "cli.h"
#include "file_reader.h"
#include "info_hash.h"
#include "torrent_metadata.h"
#include <stddef.h>
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

  bencode_object_t obj = {.type = INVALID, .value.integer = 9870};

  bool is_parsed = parse_bencode_buffer(&parser, &obj);

  if (!is_parsed) {
    fprintf(stderr, "Failed to parse file.\n");
    free_buffer(&buffer);
    return 1;
  }

  torrent_info_t torrent_info = {.name = {.data = NULL, .length = 0},
                                 .length = 0,
                                 .piece_length = 0,
                                 .pieces = {.data = NULL, .length = 0},
                                 .info_span = {.start_offset = 0, .length = 0}};

  bool info_extracted = torrent_metadata_extract_info(&obj, &torrent_info);

  if (!info_extracted) {
    fprintf(stderr, "Failed to extract info.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }
  info_hash_t info_hash = {.bytes = {0}};

  if (torrent_info.info_span.start_offset > buffer.length) {

    fprintf(stderr, "Info start offset greater than buffer length.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  if (torrent_info.info_span.length >
      buffer.length - torrent_info.info_span.start_offset) {
    fprintf(stderr, "buffer length exceeded.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  bool computed =
      compute_info_hash(&buffer.data[torrent_info.info_span.start_offset],
                        torrent_info.info_span.length, &info_hash);
  if (!computed) {
    fprintf(stderr, "Failed to compute info hash.\n");
    free_bencode_object(&obj);
    free_buffer(&buffer);
    return 1;
  }

  for (size_t i = 0; i < INFO_HASH_LENGTH; i++) {
    printf("%02x", (unsigned int)info_hash.bytes[i]);
  }

  fputc('\n', stdout);

  // free buffer when program ends.
  free_bencode_object(&obj);
  free_buffer(&buffer);

  return 0;
}
