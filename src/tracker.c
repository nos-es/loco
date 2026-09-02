#include "tracker.h"
#include "info_hash.h"
#include "peer_id.h"
#include <curl/curl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t write_chunk_to_tracker_response_buffer(char *chunk, size_t size,
                                              size_t nmemb,
                                              void *tracker_response_buffer) {
  if (tracker_response_buffer == NULL) {
    return 0;
  }

  size_t chunk_size = size * nmemb;
  tracker_response_buffer_t *response_buffer =
      (tracker_response_buffer_t *)tracker_response_buffer;

  if (response_buffer->length > (SIZE_MAX - chunk_size)) {

    return 0;
  }

  size_t new_buffer_length = response_buffer->length + chunk_size;

  unsigned char *temp_buffer =
      realloc(response_buffer->data, new_buffer_length);

  if (temp_buffer == NULL) {
    return 0;
  }
  response_buffer->data = temp_buffer;

  memcpy(response_buffer->data + response_buffer->length, chunk, chunk_size);
  response_buffer->length = new_buffer_length;

  return chunk_size;
}

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

static bool safely_increased_url_length(size_t *url_length,
                                        size_t increasing_length) {
  if (*url_length > (SIZE_MAX - increasing_length)) {

    return false;
  }
  *url_length += increasing_length;
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

  const char *event_value = "";
  bool has_event = false;

  switch (request->event) {
  case TRACKER_EVENT_NONE:
    break;
  case TRACKER_EVENT_STARTED:
    has_event = true;
    event_value = "started";
    break;
  case TRACKER_EVENT_COMPLETED:
    has_event = true;
    event_value = "completed";
    break;
  case TRACKER_EVENT_STOPPED:
    has_event = true;
    event_value = "stopped";
    break;
  default:
    // not supported event value
    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    return NULL;
  }

  size_t announce_len = announce->length;
  size_t info_hash_encoded_len = strlen(info_hash_encoded);
  size_t peer_id_encoded_len = strlen(peer_id_encoded);
  size_t port_len = strlen(port);
  size_t uploaded_len = strlen(uploaded);
  size_t downloaded_len = strlen(downloaded);
  size_t left_len = strlen(left);
  size_t event_len = strlen(event_value);

  size_t url_capacity = 1;

  const char *static_text =
      "?info_hash=&peer_id=&port=&uploaded=&downloaded=&left=&compact=1";

  if (has_event) {
    static_text = "?info_hash=&peer_id=&port=&uploaded=&downloaded=&left=&"
                  "compact=1&event=";
  }

  size_t static_text_len = strlen(static_text);

  if (!safely_increased_url_length(&url_capacity, announce_len) ||
      !safely_increased_url_length(&url_capacity, info_hash_encoded_len) ||
      !safely_increased_url_length(&url_capacity, peer_id_encoded_len) ||
      !safely_increased_url_length(&url_capacity, port_len) ||
      !safely_increased_url_length(&url_capacity, uploaded_len) ||
      !safely_increased_url_length(&url_capacity, downloaded_len) ||
      !safely_increased_url_length(&url_capacity, left_len) ||
      !safely_increased_url_length(&url_capacity, static_text_len) ||
      !safely_increased_url_length(&url_capacity, event_len)) {

    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);

    return NULL;
  }

  char *url = malloc(url_capacity);

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

  if (position >= url_capacity) {

    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    free(url);

    return NULL;
  }
  url[position] = separator;
  position++;

  if (!add_query_parameter_to_url(url, url_capacity,
                                  "info_hash=", info_hash_encoded, &position) ||
      !add_query_parameter_to_url(url, url_capacity,
                                  "&peer_id=", peer_id_encoded, &position) ||
      !add_query_parameter_to_url(url, url_capacity, "&port=", port,
                                  &position) ||
      !add_query_parameter_to_url(url, url_capacity, "&uploaded=", uploaded,
                                  &position) ||
      !add_query_parameter_to_url(url, url_capacity, "&downloaded=", downloaded,
                                  &position) ||
      !add_query_parameter_to_url(url, url_capacity, "&left=", left,
                                  &position) ||
      !add_query_parameter_to_url(url, url_capacity, "&compact=", compact,
                                  &position)) {

    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    free(url);
    return NULL;
  }

  if (has_event && !add_query_parameter_to_url(
                       url, url_capacity, "&event=", event_value, &position)) {
    curl_free(info_hash_encoded);
    curl_free(peer_id_encoded);
    curl_easy_cleanup(curl);
    free(url);
    return NULL;
  }

  if (position != url_capacity - 1) {
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
