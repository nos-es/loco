#include "info_hash.h"
#include <openssl/evp.h>

bool compute_info_hash(const unsigned char *bytes, size_t length,
                       info_hash_t *out_info_hash) {
  if (bytes == NULL || out_info_hash == NULL) {
    return false;
  }
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (ctx == NULL) {
    return false;
  }

  int init_result_code = EVP_DigestInit_ex2(ctx, EVP_sha1(), NULL);

  if (init_result_code != 1) {
    EVP_MD_CTX_free(ctx);
    return false;
  }
  int update_result_code = EVP_DigestUpdate(ctx, bytes, length);

  if (update_result_code != 1) {
    EVP_MD_CTX_free(ctx);
    return false;
  }
  info_hash_t temp_hash = {0};
  unsigned int digest_length = 0;

  int final_result_code =
      EVP_DigestFinal_ex(ctx, temp_hash.bytes, &digest_length);
  if (final_result_code != 1 || digest_length != INFO_HASH_LENGTH) {

    EVP_MD_CTX_free(ctx);
    return false;
  }

  EVP_MD_CTX_free(ctx);
  *out_info_hash = temp_hash;

  return true;
}
