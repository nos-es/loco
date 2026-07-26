#pragma once
#include <stdbool.h>

bool cli_arguments_valid(int argc, char *argv[], const char **out_torrent_path);
