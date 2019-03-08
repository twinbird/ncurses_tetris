CC=gcc
FLAGS=-lncurses

tetoris: tetoris.c
	$(CC) $(FLAGS) tetoris.c -o tetoris

.PHONY: clean
clean:
	rm -f tetoris
