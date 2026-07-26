CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude

loco:	src/main.c src/cli.c include/cli.h src/file_reader.c include/file_reader.h
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c src/cli.c src/file_reader.c -o loco

clean:	
	rm -rf loco

.PHONY:	clean
