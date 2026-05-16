CC = gcc
CFLAGS = -Wall -Wextra -fno-stack-protector

main: main.c
	$(CC) $(CFLAGS) main.c -o main

clean:
	rm -f main
