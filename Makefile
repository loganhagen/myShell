CC= gcc
CFLAGS= -g -Wpedantic -std=gnu99 -I.

shell: myShell.c main.c LinkedListAPI.c
	$(CC) $(CFLAGS) -o myShell myShell.c main.c LinkedListAPI.c

clean:
	rm myShell