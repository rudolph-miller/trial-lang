all: build run

build:
	clang -o bin/trial-lang -I ./include src/main.c src/state.c

run:
	bin/trial-lang
