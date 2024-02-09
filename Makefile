all: main tetris.wasm

main: main.o tetris.o
	clang -o main main.o tetris.o

main.o: main.c
	clang -c main.c

tetris.o: tetris.c
	clang -c tetris.c

tetris.wasm: tetris.c
	clang --target=wasm32 --no-standard-libraries -Wl,--export-all \
		-Wl,--allow-undefined -Wl,--no-entry -o tetris.wasm tetris.c

.PHONY: clean

clean:
	rm -f main main.o tetris.o
