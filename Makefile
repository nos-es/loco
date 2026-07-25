CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g

loco:	src/main.c
	$(CC) $(CFLAGS) src/main.c -o loco

clean:	
	rm -rf loco

.PHONY:	clean
