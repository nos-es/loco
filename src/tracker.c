#include "tracker.h"
#include "info_hash.h"
#include "peer_id.h"
#include <curl/curl.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char *build_tracker_url(const bencode_segment_t *announce,
                        const tracker_request_t *request) {

  if (announce == NULL || request == NULL) {
    return NULL;
  }

  CURL *curl = curl_easy_init();
  if (!curl) {
    return NULL;
  }

  char *info_hash_encoded = curl_easy_escape(
      curl, (const char *)request->info_hash.bytes, INFO_HASH_LENGTH);

  if (!info_hash_encoded) {
    curl_easy_cleanup(curl);
    return NULL;
  }

  char *peer_id_encoded = curl_easy_escape(
      curl, (const char *)request->peer_id.bytes, PEER_ID_LENGTH);

  if (!peer_id_encoded) {
    curl_free(info_hash_encoded);
    curl_easy_cleanup(curl);
    return NULL;
  }
  printf("\n");
  printf("info encoded: %s\n", info_hash_encoded);
  printf("peer_id encoded: %s\n", peer_id_encoded);

  // Convert to text
  //  Buffer to hold the resulting string
  char port[sizeof("65535")];
  snprintf(port, sizeof(port), "%" PRIu16, request->port);

  char uploaded[sizeof("18446744073709551615")];
  snprintf(uploaded, sizeof(uploaded), "%" PRIu64, request->uploaded);

  char downloaded[sizeof("18446744073709551615")];
  snprintf(downloaded, sizeof(downloaded), "%" PRIu64, request->downloaded);

  char left[sizeof("18446744073709551615")];
  snprintf(left, sizeof(left), "%" PRIu64, request->left);

  size_t announce_eln = announce->length;
  size_t info_hash_encoded_len = strlen(info_hash_encoded);
  size_t peer_id_encoded_len = strlen(peer_id_encoded);
  size_t port_len = strlen(port);
  size_t uploaded_len = strlen(uploaded);
  size_t downloaded_len = strlen(downloaded);
  size_t left_len = strlen(left);
  size_t static_text_len = strlen(
      "?info_hash=&peer_id=&port=&uploaded=&downloaded=&left=&compact=1");

  size_t url_length = announce_eln + info_hash_encoded_len +
                      peer_id_encoded_len + port_len + uploaded_len +
                      downloaded_len + left_len + static_text_len + 1;

  curl_free(info_hash_encoded);
  curl_free(peer_id_encoded);
  curl_easy_cleanup(curl);

  return NULL;
}
