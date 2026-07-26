#include "file_reader.h"
#include <stddef.h>
#include <stdio.h>

static bool get_file_size(FILE *file, size_t *out_file_size) {

  if (out_file_size == NULL) {
    return false;
  }

  *out_file_size = 0;

  int fseek_result = fseek(file, 0, SEEK_END);

  if (fseek_result != 0) {
    perror("Error while moving position in file");
    return false;
  }

  long position = ftell(file);
  if (position < 0) {
    perror("Error while reading position in file");
    return false;
  }
  size_t file_size = (size_t)position;

  fseek_result = fseek(file, 0, SEEK_SET);

  if (fseek_result != 0) {
    perror("Error while moving position back to 0 in file");
    return false;
  }

  *out_file_size = file_size;

  return true;
}

bool read_byte_buffer_from_file(const char *torrent_filepath,
                                byte_buffer_t *out_buffer) {

  if (torrent_filepath == NULL || out_buffer == NULL) {
    fprintf(stderr, "Torrent filepath or provided buffer was null\n");
    return false;
  }
  out_buffer->data = NULL;
  out_buffer->length = 0;

  FILE *file = fopen(torrent_filepath, "rb");

  if (file == NULL) {
    perror("Torrent file could not be opened");
    return false;
  }

  printf("Torrent file opened: %s\n", torrent_filepath);

  size_t file_size = 0;

  bool file_size_obtained = get_file_size(file, &file_size);

  if (!file_size_obtained) {
    fclose(file);
    printf("Closed Torrent file\n");
    return false;
  }

  printf("Torrent file size: %zu bytes\n", file_size);

  fclose(file);
  printf("Closed Torrent file\n");
  // returns false, while working on this function..
  return false;
}
