CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude
SANFLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
TEST_CPPFLAGS	=	-Ivendor/munit

loco:	src/main.c src/cli.c include/cli.h src/file_reader.c include/file_reader.h src/bencode_parser.c include/bencode_parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c src/cli.c src/file_reader.c src/bencode_parser.c -o loco

sanitize:	src/main.c src/cli.c include/cli.h src/file_reader.c include/file_reader.h src/bencode_parser.c include/bencode_parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANFLAGS) src/main.c src/cli.c src/file_reader.c src/bencode_parser.c -o loco

test: tests/test_bencode_parser.c	src/bencode_parser.c include/bencode_parser.h vendor/munit/munit.c vendor/munit/munit.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_CPPFLAGS) tests/test_bencode_parser.c src/bencode_parser.c vendor/munit/munit.c -o loco-tests
	./loco-tests

clean:	
	rm -rf loco loco-tests

.PHONY:	clean sanitize test
