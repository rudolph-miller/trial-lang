all: build run

build:
	clang -o bin/trial-lang -I ./include src/main.c src/state.c src/gc.c src/pair.c src/write.c src/symbol.c src/value.c

clean:
	rm bin/trial-lang

run:
	bin/trial-lang
