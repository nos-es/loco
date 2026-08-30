#include "tracker.h"
#include "info_hash.h"
#include "peer_id.h"
#include <curl/curl.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool add_query_parameter_to_url(char *url, size_t capacity,
                                       const char *parameter_prefix,
                                       const char *parameter_value,
                                       size_t *position) {

  if (*position > capacity) {
    return false;
  }
  size_t remaining = capacity - *position;
  size_t parameter_prefix_length = strlen(parameter_prefix);
  size_t parameter_value_length = strlen(parameter_value);

  if (parameter_prefix_length > remaining) {
    return false;
  }

  remaining -= parameter_prefix_length;

  if (parameter_value_length > remaining) {
    return false;
  }

  memcpy(url + *position, parameter_prefix, parameter_prefix_length);
  *position += parameter_prefix_length;

  memcpy(url + *position, parameter_value, parameter_value_length);
  *position += parameter_value_length;
  return true;
}

char *build_tracker_url(const bencode_segment_t *announce,
                        const tracker_request_t *request) {

  if (announce == NULL || request == NULL) {
    return NULL;
  }

  if (!(announce->data != NULL && announce->length > 0)) {
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

  // Convert to text
  //  Buffer to hold the resulting string
  char port[sizeof("65535")];
  int port_written = snprintf(port, sizeof(port), "%" PRIu16, request->port);

  if (port_written < 0 || port_written >= (int)sizeof(port)) {
    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    return NULL;
  }

  char uploaded[sizeof("18446744073709551615")];
  int upload_written =
      snprintf(uploaded, sizeof(uploaded), "%" PRIu64, request->uploaded);

  if (upload_written < 0 || upload_written >= (int)sizeof(uploaded)) {
    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    return NULL;
  }

  char downloaded[sizeof("18446744073709551615")];
  int downloaded_written =
      snprintf(downloaded, sizeof(downloaded), "%" PRIu64, request->downloaded);

  if (downloaded_written < 0 || downloaded_written >= (int)sizeof(downloaded)) {
    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    return NULL;
  }

  char left[sizeof("18446744073709551615")];
  int left_written = snprintf(left, sizeof(left), "%" PRIu64, request->left);

  if (left_written < 0 || left_written >= (int)sizeof(left)) {
    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    return NULL;
  }

  const char *compact = "0";
  if (request->compact) {
    compact = "1";
  }

  size_t announce_len = announce->length;
  size_t info_hash_encoded_len = strlen(info_hash_encoded);
  size_t peer_id_encoded_len = strlen(peer_id_encoded);
  size_t port_len = strlen(port);
  size_t uploaded_len = strlen(uploaded);
  size_t downloaded_len = strlen(downloaded);
  size_t left_len = strlen(left);
  size_t static_text_len = strlen(
      "?info_hash=&peer_id=&port=&uploaded=&downloaded=&left=&compact=1");

  size_t url_length = announce_len + info_hash_encoded_len +
                      peer_id_encoded_len + port_len + uploaded_len +
                      downloaded_len + left_len + static_text_len;

  char *url = malloc(url_length + 1);

  if (url == NULL) {
    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);

    return NULL;
  }
  size_t position = 0;

  memcpy(url, announce->data, announce->length);
  position += announce->length;

  char separator = '&';

  if (memchr(url, '?', position) == NULL) {
    separator = '?';
  }

  if (position >= url_length) {

    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    free(url);

    return NULL;
  }
  url[position] = separator;
  position++;

  if (!add_query_parameter_to_url(url, url_length,
                                  "info_hash=", info_hash_encoded, &position) ||
      !add_query_parameter_to_url(url, url_length, "&peer_id=", peer_id_encoded,
                                  &position) ||
      !add_query_parameter_to_url(url, url_length, "&port=", port, &position) ||
      !add_query_parameter_to_url(url, url_length, "&uploaded=", uploaded,
                                  &position) ||
      !add_query_parameter_to_url(url, url_length, "&downloaded=", downloaded,
                                  &position) ||
      !add_query_parameter_to_url(url, url_length, "&left=", left, &position) ||
      !add_query_parameter_to_url(url, url_length, "&compact=", compact,
                                  &position)) {

    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    free(url);
    return NULL;
  }

  url[position] = '\0';

  curl_free(info_hash_encoded);
  curl_free(peer_id_encoded);
  curl_easy_cleanup(curl);

  return url;
}
