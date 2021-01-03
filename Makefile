.PHONY: all build clean

all: build

build:
	@gcc -Wall basic_movements.c -o basic_movements -lncurses

clean:
	@rm basic_movements
