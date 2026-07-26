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
  fclose(file);
  printf("Closed Torren file");

  return 0;
}
