#include "tracker.h"
#include "bencode_parser.h"
#include "bencode_types.h"
#include "info_hash.h"
#include "peer_id.h"
#include <arpa/inet.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/typecheck-gcc.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

static const unsigned char interval_key[] = "interval";
static const unsigned char peers_key[] = "peers";
static const unsigned char ip_key[] = "ip";
static const unsigned char port_key[] = "port";

static bool is_valid_peers_ipv4_list(const bencode_segment_t *peers_segment) {

  if (peers_segment == NULL) {
    return false;
  }

  if (peers_segment->length % PEER_SEGMENT_LENGTH != 0) {
    return false;
  }

  if (peers_segment->length > 0 && peers_segment->data == NULL) {
    return false;
  }

  return true;
}

bool peers_extract(const bencode_segment_t *peers_segment, peer_t **out_peers,
                   size_t *out_peer_count) {

  if (peers_segment == NULL || out_peers == NULL || out_peer_count == NULL) {
    return false;
  }

  if (!is_valid_peers_ipv4_list(peers_segment)) {
    return false;
  }
  size_t peer_count = peers_segment->length / PEER_SEGMENT_LENGTH;

  // Currently no peers.
  if (peer_count == 0) {
    *out_peer_count = 0;
    *out_peers = NULL;
    return true;
  }

  // size_t overflow check
  if (peer_count > SIZE_MAX / sizeof(peer_t)) {
    return false;
  }
  peer_t *peers = malloc(peer_count * sizeof(peer_t));

  if (peers == NULL) {
    return false;
  }

  for (size_t i = 0; i < peer_count; i++) {

    size_t offset = i * PEER_SEGMENT_LENGTH;
    peer_t current_peer = {0};
    bencode_segment_t current_segment = {.data = peers_segment->data + offset,
                                         .length = PEER_SEGMENT_LENGTH};

    bool parsed_peer = parse_peer_from_segment(&current_segment, &current_peer);

    if (!parsed_peer) {
      free(peers);
      return false;
    }
    peers[i] = current_peer;
  }

  *out_peers = peers;
  *out_peer_count = peer_count;

  return true;
}

bool parse_peer_from_segment(const bencode_segment_t *peer_segment,
                             peer_t *out_peer) {

  if (peer_segment == NULL || out_peer == NULL) {
    return false;
  }
  if (peer_segment->data == NULL ||
      peer_segment->length != PEER_SEGMENT_LENGTH) {
    return false;
  }

  peer_t temp_peer = {0};
  memcpy(temp_peer.ipv4_address, peer_segment->data, PEER_IP_LENGTH);
  uint16_t high = (uint16_t)peer_segment->data[4];
  uint16_t low = (uint16_t)peer_segment->data[5];
  uint16_t temp_port = (high << 8) | low;
  temp_peer.port = temp_port;

  *out_peer = temp_peer;
  return true;
}

static bool is_ipv4(const unsigned char ip_buffer, const size_t buffer_length) {
}

static const bencode_object_t *
find_entry_in_dictionary(const bencode_object_t *root,
                         const unsigned char *key_name, size_t key_length,
                         bencode_data_type_t target_type) {
  if (root == NULL) {
    return NULL;
  }
  if (root->type != DICTIONARY) {
    return NULL;
  }
  for (size_t i = 0; i < root->value.dictionary.count; i++) {

    const bencode_dictionary_entry_t *current_entry =
        &root->value.dictionary.entries[i];

    if (current_entry->key.length != key_length) {
      continue;
    }

    int compare_result =
        memcmp(current_entry->key.data, key_name, current_entry->key.length);
    if (compare_result != 0) {
      continue;
    }

    if (current_entry->value.type != target_type) {
      continue;
    }

    return &current_entry->value;
  }
  return NULL;
}

bool find_interval(const bencode_object_t *response_obj,
                   int64_t *out_interval) {
  if (out_interval == NULL) {
    return false;
  }

  const bencode_object_t *entry = find_entry_in_dictionary(
      response_obj, interval_key, sizeof(interval_key) - 1, INTEGER);

  if (entry == NULL) {
    return false;
  }
  *out_interval = entry->value.integer;
  return true;
}

bool find_peers(const bencode_object_t *response_obj,
                bencode_segment_t *out_peers) {
  if (out_peers == NULL) {
    return false;
  }

  // compact ip list
  const bencode_object_t *entry = find_entry_in_dictionary(
      response_obj, peers_key, sizeof(peers_key) - 1, BYTE_STRING);

  if (entry != NULL) {
    *out_peers = entry->value.byte_string;
    return true;
  }

  // handle non compact ip list
  const bencode_object_t *list_entry = find_entry_in_dictionary(
      response_obj, peers_key, sizeof(peers_key) - 1, LIST);

  if (list_entry == NULL) {
    return false;
  }

  // TODO: format to bencode_segment_t
  if (list_entry->type == LIST) {
    for (size_t i = 0; i < list_entry->value.list.count; i++) {
      bencode_object_t list_item = list_entry->value.list.items[i];
      if (list_item.type != DICTIONARY) {
        continue;
      }
      for (size_t j = 0; j < list_item.value.dictionary.count; j++) {

        bencode_dictionary_entry_t entry =
            list_item.value.dictionary.entries[j];

        const unsigned char *key = entry.key.data;
        if (entry.key.length != sizeof(ip_key) - 1) {
          continue;
        }

        int is_ip_key = memcmp(key, ip_key, sizeof(ip_key) - 1);

        if (is_ip_key == 0) {
          if (entry.value.type != BYTE_STRING) {
            continue;
          }

          char ip[entry.value.value.byte_string.length + 1];

          memcpy(ip, entry.value.value.byte_string.data,
                 entry.value.value.byte_string.length);

          ip[entry.value.value.byte_string.length] = '\0';
          struct in_addr address;
          if (inet_pton(AF_INET, ip, &address) != 1) {
            continue;
          }
        }
      }
    }
  }
  return false;
}

bool tracker_announce(const bencode_segment_t *announce,
                      const tracker_request_t *request,
                      tracker_response_buffer_t *out_response) {

  if (announce == NULL || request == NULL || out_response == NULL) {
    return false;
  }

  CURL *curl = curl_easy_init();
  tracker_response_buffer_t temp_response = {.data = NULL, .length = 0};

  if (curl == NULL) {
    return false;
  }

  CURLcode write_function_code = curl_easy_setopt(
      curl, CURLOPT_WRITEFUNCTION, write_chunk_to_tracker_response_buffer);

  if (write_function_code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return false;
  }

  CURLcode write_data_code =
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &temp_response);

  if (write_data_code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return false;
  }
  char *tracker_url = build_tracker_url(announce, request);

  if (tracker_url == NULL) {
    curl_easy_cleanup(curl);
    return false;
  }

  CURLcode set_url_code = curl_easy_setopt(curl, CURLOPT_URL, tracker_url);

  if (set_url_code != CURLE_OK) {
    curl_easy_cleanup(curl);
    free(tracker_url);
    return false;
  }
  // free, since after sucessfully set CURLOPT_URL, not needed.
  free(tracker_url);

  CURLcode easy_perform_code = curl_easy_perform(curl);

  if (easy_perform_code != CURLE_OK) {
    curl_easy_cleanup(curl);
    free(temp_response.data);
    return false;
  }

  long status_code = 0;
  CURLcode info_code =
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

  if (info_code != CURLE_OK || status_code != 200) {
    curl_easy_cleanup(curl);
    free(temp_response.data);
    return false;
  }

  *out_response = temp_response;
  curl_easy_cleanup(curl);
  return true;
}
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

bool tracker_response_parse(const tracker_response_buffer_t *response,
                            bencode_object_t *out_parsed_obj) {

  if (response == NULL || out_parsed_obj == NULL) {
    return false;
  }

  parser_state_t parser;
  bool parse_result =
      bencode_parser_init(&parser, response->data, response->length);

  if (!parse_result) {
    return false;
  }
  bencode_object_t obj = {.type = INVALID};

  bool is_parsed = parse_bencode_buffer(&parser, &obj);

  if (!is_parsed) {
    return false;
  }

  if (parser.position != response->length) {
    free_bencode_object(&obj);
    return false;
  }

  if (obj.type != DICTIONARY) {
    free_bencode_object(&obj);
    return false;
  }

  *out_parsed_obj = obj;
  return true;
}
