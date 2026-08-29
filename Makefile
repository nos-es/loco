CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude
SANFLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
TEST_CPPFLAGS	=	-Ivendor/munit
LDLIBS=	-lcrypto

loco:	src/main.c src/cli.c include/cli.h src/file_reader.c include/file_reader.h src/bencode_parser.c include/bencode_parser.h src/torrent_metadata.c include/torrent_metadata.h src/info_hash.c include/info_hash.h src/peer_id.c include/peer_id.h
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c src/cli.c src/file_reader.c src/bencode_parser.c src/torrent_metadata.c src/info_hash.c src/peer_id.c -o loco $(LDLIBS)

sanitize:	src/main.c src/cli.c include/cli.h src/file_reader.c include/file_reader.h src/bencode_parser.c include/bencode_parser.h src/torrent_metadata.c include/torrent_metadata.h src/info_hash.c include/info_hash.h src/peer_id.c include/peer_id.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANFLAGS) src/main.c src/cli.c src/file_reader.c src/bencode_parser.c src/torrent_metadata src/info_hash.c src/peer_id.c -o loco $(LDLIBS)

test: tests/test_main.c tests/test_bencode_parser.c	src/bencode_parser.c include/bencode_parser.h tests/test_torrent_metadata.c src/torrent_metadata.c include/torrent_metadata.h tests/test_info_hash.c src/info_hash.c include/info_hash.h tests/test_peer_id.c src/peer_id.c include/peer_id.h vendor/munit/munit.c vendor/munit/munit.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_CPPFLAGS) tests/test_main.c tests/test_bencode_parser.c src/bencode_parser.c tests/test_torrent_metadata.c src/torrent_metadata.c tests/test_info_hash.c src/info_hash.c tests/test_peer_id.c src/peer_id.c vendor/munit/munit.c -o loco-tests $(LDLIBS)
	./loco-tests

test-sanitize: tests/test_main.c tests/test_bencode_parser.c	src/bencode_parser.c include/bencode_parser.h tests/test_torrent_metadata.c src/torrent_metadata.c include/torrent_metadata.h tests/test_info_hash.c src/info_hash.c include/info_hash.h tests/test_peer_id.c src/peer_id.c include/peer_id.h vendor/munit/munit.c vendor/munit/munit.h 
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_CPPFLAGS) $(SANFLAGS) tests/test_main.c tests/test_bencode_parser.c src/bencode_parser.c tests/test_torrent_metadata.c src/torrent_metadata.c tests/test_info_hash.c src/info_hash.c tests/test_peer_id.c src/peer_id.c vendor/munit/munit.c -o loco-tests $(LDLIBS)
	./loco-tests

clean:	
	rm -rf loco loco-tests

.PHONY:	clean sanitize test test-sanitize
