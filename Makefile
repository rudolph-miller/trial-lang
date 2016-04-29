all: build run

build:
	clang -o bin/trial-lang src/main.c

run:
	bin/trial-lang
