#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>

#define BOARD_WIDTH sizeof(uint8_t) * 8
#ifndef BOARD_HEIGHT
#define BOARD_HEIGHT 12
#endif
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

#ifdef TETRIS_IMPLEMENTATION

const uint32_t pieces[] = {
    0 | (0 << 8) | ((uint32_t)0b00011000 << 16) |
        ((uint32_t)0b00011000 << 24), // O
    0b00001000 | ((uint32_t)0b00001000 << 8) | ((uint32_t)0b00001000 << 16) |
        ((uint32_t)0b00001000 << 24), // I
    0 | (0 << 8) | ((uint32_t)0b00011000 << 16) |
        ((uint32_t)0b00001100 << 24), // S
    0 | (0 << 8) | ((uint32_t)0b00011000 << 16) |
        ((uint32_t)0b00110000 << 24), // Z
    0 | ((uint32_t)0b00010000 << 8) | ((uint32_t)0b00010000 << 16) |
        ((uint32_t)0b00011000 << 24), // L
    0 | ((uint32_t)0b00001000 << 8) | ((uint32_t)0b00001000 << 16) |
        ((uint32_t)0b00011000 << 24), // J
    0 | (0 << 8) | ((uint32_t)0b00011100 << 16) |
        ((uint32_t)0b00001000 << 24), // T
};
#define PIECE_COUNT (sizeof(pieces) / sizeof(pieces[0]))

int tetris_init(struct tetris *t) {
    for (int i = 0; i < BOARD_HEIGHT + BOARD_BUFFER; i++) {
        t->board[i] = 0;
    }
    t->score = 0;

    return 0;
}

int tetris_spawn(struct tetris *t) {
    uint32_t piece = pieces[tetris_rand_int(PIECE_COUNT)];

    int is_occupied = (t->board[0] & piece) | (t->board[1] & (piece >> 8)) |
                      (t->board[2] & (piece >> 16)) |
                      (t->board[3] & (piece >> 24));

    for (unsigned int i = 0; i < BOARD_BUFFER; i++) {
        t->board[i] = (piece >> (i * 8)) & 0b11111111;
    }

    return is_occupied;
}

int tetris_tick(struct tetris *t, enum move m) {
    int can_move = 1;

    for (int i = BOARD_HEIGHT + BOARD_BUFFER - 2; i >= 0; i--) {
        uint8_t free = (t->board[i] ^ t->board[i + 1]) & t->board[i];

        t->board[i] = t->board[i] & (~free);

        unsigned char tmp = free;
        switch (m) {
        case MOVE_LEFT: {
            if (can_move) {
                uint8_t mask = 0;
                if (tmp & (1 << (BOARD_WIDTH - 1))) {
                    mask = 1;
                }
                tmp = tmp << 1 | mask;
            }
            break;
        }
        case MOVE_RIGHT: {
            if (can_move) {
                uint8_t mask = 0;
                if (tmp & 1) {
                    mask = 1 << (BOARD_WIDTH - 1);
                }
                tmp = tmp >> 1 | mask;
            }
            break;
        }
        case MOVE_NONE:
            break;
        }

        if (can_move) {
            int blocked = t->board[i + 1] & tmp & 0b11111111;

            if (blocked) {
                can_move = 0;
            } else {
                free = tmp;
            }
        }

        t->board[i + 1] = t->board[i + 1] | free;
    }

    uint8_t mask = 0b11111111;
    uint32_t count = 0;
    while (t->board[BOARD_HEIGHT + BOARD_BUFFER - 1] == mask) {
        count++;
        for (unsigned int i = BOARD_HEIGHT + BOARD_BUFFER - 1; i > BOARD_BUFFER; i--) {
            t->board[i] = t->board[i - 1];
        }
    }
    t->score += count * count;

    return 0;
}

#endif
