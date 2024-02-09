main: main.o tetris.o
	clang -o main main.o tetris.o

main.o: main.c
	clang -c main.c

tetris.o: tetris.c
	clang -c tetris.c

.PHONY: clean

clean:
	rm -f main main.o tetris.o
