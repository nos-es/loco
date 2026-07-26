#include <stdbool.h>
#include <string.h>

bool has_torrent_extension(char *torrent_path) {

  const char *dot = strchr(torrent_path, '.');

  if (!dot || dot == torrent_path) {
    return false;
  }
  return strcmp(dot, ".torrent") == 0;
}

bool cli_arguments_valid(int argc, char *argv[],
                         const char **out_torrent_path) {

  if (out_torrent_path == NULL) {
    return false;
  }

  if (argc != 2) {
    return false;
  }

  if (!has_torrent_extension(argv[1])) {
    return false;
  }

  *out_torrent_path = argv[1];

  return true;
}
