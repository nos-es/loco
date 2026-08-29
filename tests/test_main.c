#include "munit.h"

extern const MunitSuite bencode_parser_suite;
extern const MunitSuite torrent_metadata_suite;
extern const MunitSuite info_hash_suite;
extern const MunitSuite peer_id_suite;

int main(int argc, char *argv[]) {

  MunitSuite sub_suite[] = {bencode_parser_suite,
                            torrent_metadata_suite,
                            info_hash_suite,
                            peer_id_suite,
                            {NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE}};

  const MunitSuite root_suite = {.prefix = "",
                                 .tests = NULL,
                                 .suites = sub_suite,
                                 .iterations = 1,
                                 .options = MUNIT_SUITE_OPTION_NONE};

  return munit_suite_main(&root_suite, NULL, argc, argv);
}
