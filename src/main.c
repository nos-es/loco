#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <torrent-file>\n", argv[0]);
    return 1;
  }
  printf("Provided argument: %s\n", argv[1]);
  return 0;
}
