#include "munit.h"
#include "peer_id.h"
#include <stddef.h>
#include <stdint.h>

static MunitResult
test_generate_peer_id_returns_correct_prefix(const MunitParameter params[],
                                             void *user_data) {
  (void)params;
  (void)user_data;
  peer_id_t peer_id = {0};
  bool peer_id_generated = generate_peer_id(&peer_id);

  munit_assert_true(peer_id_generated);
  munit_assert_memory_equal(PEER_ID_PREFIX_LENGTH, peer_id.bytes,
                            PEER_ID_PREFIX);

  return MUNIT_OK;
}
static MunitResult
test_generate_peer_id_reject_null_input(const MunitParameter params[],
                                        void *user_data) {
  (void)params;
  (void)user_data;
  bool peer_id_generated = generate_peer_id(NULL);

  munit_assert_false(peer_id_generated);

  return MUNIT_OK;
}

static MunitTest tests[] = {
    {"/generate_peer_id/returns-correct-prefix",
     test_generate_peer_id_returns_correct_prefix, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {"/generate_peer_id/rejects-null-input",
     test_generate_peer_id_reject_null_input, NULL, NULL,
     MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite peer_id_suite = {"/peer_id", tests, NULL, 1,
                                  MUNIT_SUITE_OPTION_NONE};
