#include "cli.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

  const char *torrent_filepath = NULL;

  if (!cli_arguments_valid(argc, argv, &torrent_filepath)) {
    fprintf(stderr, "Usage: %s <torrent-file>\n", argv[0]);
    return 1;
  }

  FILE *file = fopen(torrent_filepath, "rb");

  if (file == NULL) {
    perror("Torrent file could not be opened");
    return 1;
  }

  printf("Torrent file opened: %s\n", torrent_filepath);
  int fseek_result = fseek(file, 0, SEEK_END);

  if (fseek_result != 0) {
    perror("Error while moving position in file");
    fclose(file);
    return 1;
  }

  long position = ftell(file);
  if (position < 0) {
    perror("Error while reading position in file");
    fclose(file);
    return 1;
  }
  size_t file_size = (size_t)position;
  printf("Current position in file: %ld\n", position);
  printf("Torrent file size: %zu bytes\n", file_size);
  rewind(file);

  fclose(file);
  printf("Closed Torrent file\n");

  return 0;
}
