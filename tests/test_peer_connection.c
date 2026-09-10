#include "munit.h"
#include <stddef.h>
#include <stdint.h>

static MunitTest tests[] = {
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};

const MunitSuite peer_connection_suite = {"/peer_connection", tests, NULL, 1,
                                          MUNIT_SUITE_OPTION_NONE};
