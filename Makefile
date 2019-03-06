CC=gcc
FLAGS=-lncurses

tetoris:
	$(CC) $(FLAGS) tetoris.c -o tetoris

.PHONY: clean
clean:
	rm -f tetoris
