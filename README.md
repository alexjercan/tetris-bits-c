# Tetris in WASM

Simple clone of the tetris game in C.

The main challenge of this project is trying to make the game as memory
efficient as possible. The game has a 12x8 board which is stored in only 12
bytes using a `char[12]`. Also all the pieces are stored as `int`. The goal of
the project is to learn bitwise operations and understand the memory layout.

## Quickstart

Use `a-d` to move and `q` to quit. No rotation or fast move supported.

```console
make
python -m http.server
# open the browser at localhost:8000
```
