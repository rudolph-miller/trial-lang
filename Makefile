all: build run

build:
	clang -o bin/trial-lang -I ./include src/main.c src/state.c src/gc.c

clean:
	rm bin/trial-lang

run:
	bin/trial-lang
