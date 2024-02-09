#include "tetris.h"

const int pieces[] = {
    0 | (0 << 8) | (0b00011000 << 16) | (0b00011000 << 24),          // O
    0b00001000 | (0b00001000 << 8) | (0b00001000 << 16) |
        (0b00001000 << 24),                                          // I
    0 | (0 << 8) | (0b00011000 << 16) | (0b00001100 << 24),          // S
    0 | (0 << 8) | (0b00011000 << 16) | (0b00110000 << 24),          // Z
    0 | (0b00010000 << 8) | (0b00010000 << 16) | (0b00011000 << 24), // L
    0 | (0b00001000 << 8) | (0b00001000 << 16) | (0b00011000 << 24), // J
    0 | (0 << 8) | (0b00011100 << 16) | (0b00001000 << 24),          // T
};
#define PIECE_COUNT (sizeof(pieces) / sizeof(pieces[0]))

void tetris_init(struct tetris *t) {
    for (int i = 0; i < BOARD_HEIGHT + BOARD_BUFFER; i++) {
        t->board[i] = 0;
    }
    t->score = 0;
}

int tetris_spawn(struct tetris *t) {
    int piece = pieces[tetris_rand_int() % PIECE_COUNT];

    int is_occupied = t->board[0] & piece | t->board[1] & (piece >> 8) |
                      t->board[2] & (piece >> 16) | t->board[3] & (piece >> 24);

    for (int i = 0; i < BOARD_BUFFER; i++) {
        t->board[i] = (piece >> (i * 8)) & 0b11111111;
    }

    return is_occupied;
}

void tetris_tick(struct tetris *t, enum move m) {
    int can_move = 1;

    for (int i = BOARD_HEIGHT + BOARD_BUFFER - 2; i >= 0; i--) {
        unsigned char free = (t->board[i] ^ t->board[i + 1]) & t->board[i];

        t->board[i] = t->board[i] & (~free);

        unsigned char tmp = free;
        switch (m) {
        case MOVE_LEFT: {
            if (can_move) {
                unsigned char mask = 0;
                if (tmp & (1 << (BOARD_WIDTH - 1))) {
                    mask = 1;
                }
                tmp = tmp << 1 | mask;
            }
            break;
        }
        case MOVE_RIGHT: {
            if (can_move) {
                unsigned char mask = 0;
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

    unsigned char mask = 0b11111111;
    int count = 0;
    while (t->board[BOARD_HEIGHT + BOARD_BUFFER - 1] == mask) {
        count++;
        for (int i = BOARD_HEIGHT + BOARD_BUFFER - 1; i > BOARD_BUFFER; i--) {
            t->board[i] = t->board[i - 1];
        }
    }
    t->score += count * count;
}