.PHONY: all build clean

all: build

build:
	@gcc -Wall basic_movements.c -o basic_movements -lncurses
	@gcc -Wall -pthread tetris_ascii.c -o tetris_ascii -lncurses

clean:
	@rm -f basic_movements tetris_ascii
