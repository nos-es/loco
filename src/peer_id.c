#include "peer_id.h"
#include <openssl/rand.h>
#include <stddef.h>

bool generate_peer_id(peer_id_t *out_peer_id) {

  if (out_peer_id == NULL) {
    return false;
  }

  peer_id_t temp_peer_id = {.bytes = PEER_ID_PREFIX};

  int return_code = RAND_bytes(temp_peer_id.bytes + PEER_ID_PREFIX_LENGTH,
                               PEER_ID_LENGTH - PEER_ID_PREFIX_LENGTH);

  if (return_code != 1) {
    return false;
  }

  *out_peer_id = temp_peer_id;

  return true;
}
