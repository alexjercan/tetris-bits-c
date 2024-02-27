#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>

#define BOARD_WIDTH sizeof(uint8_t) * 8
#define BOARD_HEIGHT 12
#define BOARD_BUFFER 4

enum move {
    MOVE_LEFT = 0,
    MOVE_RIGHT,
    MOVE_NONE,
};

struct tetris {
        uint32_t score;
        uint8_t board[BOARD_HEIGHT + BOARD_BUFFER];
};

int tetris_rand_int(int max);

int tetris_init(struct tetris *t);
int tetris_spawn(struct tetris *t);
int tetris_tick(struct tetris *t, enum move m);

#endif
