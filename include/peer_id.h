
enum { PEER_ID_LENGTH = 20 };
typedef struct PeerId {
  unsigned char bytes[PEER_ID_LENGTH];
} peer_id_t;
