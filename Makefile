CC=gcc
FLAGS=-lncurses

tetoris: tetris.c
	$(CC) $(FLAGS) tetris.c -o tetris

.PHONY: clean
clean:
	rm -f tetris
