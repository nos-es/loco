#include "cli.h"
#include "file_reader.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

  const char *torrent_filepath = NULL;

  if (!cli_arguments_valid(argc, argv, &torrent_filepath)) {
    fprintf(stderr, "Usage: %s <torrent-file>\n", argv[0]);
    return 1;
  }

  byte_buffer_t buffer = {.data = NULL, .length = 0};
  bool result = read_byte_buffer_from_file(torrent_filepath, &buffer);

  return 0;
}
