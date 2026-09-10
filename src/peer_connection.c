#include "tracker.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int connect_to_peer(const peer_t *peer) {

  if (peer == NULL) {
    return -1;
  }

  int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

  if (file_descriptor < 0) {
    return -1;
  }

  struct sockaddr_in socket_address = {.sin_family = AF_INET,
                                       .sin_port = htons(peer->port)};

  memcpy(&socket_address.sin_addr, peer->ipv4_address, PEER_IP_LENGTH);

  int connect_result =
      connect(file_descriptor, (const struct sockaddr *)&socket_address,
              sizeof(socket_address));

  if (connect_result == -1) {
    close(file_descriptor);
    return -1;
  }

  return file_descriptor;
}
