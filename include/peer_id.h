#pragma once
#include <stdbool.h>

#define PEER_ID_PREFIX "-LO0001-"

enum {
  PEER_ID_LENGTH = 20,
  PEER_ID_PREFIX_LENGTH = sizeof(PEER_ID_PREFIX) - 1
};

typedef struct PeerId {
  unsigned char bytes[PEER_ID_LENGTH];
} peer_id_t;

bool generate_peer_id(peer_id_t *out_peer_id);
