CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude

loco:	src/main.c src/cli.c include/cli.h
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c src/cli.c -o loco

clean:	
	rm -rf loco

.PHONY:	clean
