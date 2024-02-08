#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
#define BOARD_WIDTH sizeof(unsigned char) * 8
#define BOARD_HEIGHT 12
#define BOARD_BUFFER 4

// GLOBALS, I KNOW, I KNOW
int score = 0;
int is_game_over = 0;
char input;

const int pieces[] = {
    0 | (0 << 8) | (0b00011000 << 16) | (0b00011000 << 24), // O
    0b00001000 | (0b00001000 << 8) | (0b00001000 << 16) |
        (0b00001000 << 24),                                          // I
    0 | (0 << 8) | (0b00011000 << 16) | (0b00001100 << 24),          // S
    0 | (0 << 8) | (0b00011000 << 16) | (0b00110000 << 24),          // I
    0 | (0b00010000 << 8) | (0b00010000 << 16) | (0b00011000 << 24), // L
    0 | (0b00001000 << 8) | (0b00001000 << 16) | (0b00011000 << 24), // J
    0 | (0 << 8) | (0b00011100 << 16) | (0b00001000 << 24),          // T
};
#define PIECE_COUNT (sizeof(pieces) / sizeof(pieces[0]))

enum move {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_NONE,
};

struct tetris {
        unsigned char board[BOARD_HEIGHT + BOARD_BUFFER];
};

void tetris_init(struct tetris *t) {
    memset(t->board, 0, (BOARD_HEIGHT + BOARD_BUFFER) * sizeof(unsigned char));
}

int tetris_spawn(struct tetris *t) {
    int piece = pieces[rand() % PIECE_COUNT];

    int is_occupied = t->board[0] & piece | t->board[1] & (piece >> 8) |
                      t->board[2] & (piece >> 16) | t->board[3] & (piece >> 24);

    memcpy(t->board, &piece, sizeof(unsigned char) * BOARD_BUFFER);

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
    score += count * count;
}

void tetris_print(struct tetris *t) {
    for (int i = BOARD_BUFFER; i < BOARD_HEIGHT + BOARD_BUFFER; i++) {
        unsigned char line = t->board[i];

        for (int j = 0; j < BOARD_WIDTH; j++) {
            unsigned char mask = 1 << j;
            printf("%c", (line & mask) ? 'O' : ' ');
        }
        printf("\n");
    }
}

void *input_thread(void *arg) {
    for (;;) {
        input = getchar();

        if (input == 'q') {
            break;
        }
    }

    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t input_thread_id;
    pthread_create(&input_thread_id, NULL, input_thread, NULL);

    struct tetris t;
    tetris_init(&t);

    int i = 0;
    while (1) {
        struct tetris tmp = t;

        enum move m = MOVE_NONE;
        if (input == 'q') {
            break;
        } else if (input == 'd') {
            m = MOVE_LEFT;
        } else if (input == 'a') {
            m = MOVE_RIGHT;
        }
        input = 0;
        tetris_tick(&t, m);

        if (memcmp(tmp.board, t.board,
                   (BOARD_HEIGHT + BOARD_BUFFER) * sizeof(unsigned char)) ==
            0) {
            is_game_over = tetris_spawn(&t);
        }

        system("/bin/stty cooked");
        printf("%s", CLEAR_SCREEN_ANSI);
        tetris_print(&t);
        if (is_game_over) {
            printf("Game over\n");
            break;
        }
        system("/bin/stty raw");

        usleep(160000);
    }

    if (is_game_over) {
        pthread_cancel(input_thread_id);
    } else {
        pthread_join(input_thread_id, NULL);
    }

    printf("\nScore: %d\n", score);

    return 0;
}
