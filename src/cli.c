#include <stdbool.h>
#include <stdio.h>

bool cli_arguments_valid(int argc, char *argv[], const char **out_torrent_path) {

  if (out_torrent_path == NULL) {
    return false;
  }

  if (argc != 2) {
    return false;
  }

  *out_torrent_path = argv[1];

  return true;
}
