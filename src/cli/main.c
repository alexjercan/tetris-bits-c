#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tetris.h"

int tetris_rand_int(int max) {
    return rand() % max;
}

const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";

// GLOBALS, I KNOW, I KNOW
char input;

void tetris_print(struct tetris *t) {
    for (int i = BOARD_BUFFER; i < BOARD_HEIGHT + BOARD_BUFFER; i++) {
        unsigned char line = t->board[i];

        for (int j = BOARD_WIDTH - 1; j >= 0; j--) {
            unsigned char mask = 1 << j;
            printf("%c", (line & mask) ? 'O' : ' ');
        }
        printf("\n");
    }
}

void *input_thread() {
    for (;;) {
        input = getchar();

        if (input == 'q') {
            break;
        }
    }

    return NULL;
}

int main() {
    int is_game_over = 0;
    srand(time(NULL));

    pthread_t input_thread_id;
    pthread_create(&input_thread_id, NULL, input_thread, NULL);

    struct tetris t;
    tetris_init(&t);

    while (1) {
        struct tetris tmp = t;

        enum move m = MOVE_NONE;
        if (input == 'q') {
            break;
        } else if (input == 'a') {
            m = MOVE_LEFT;
        } else if (input == 'd') {
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

    printf("\nScore: %d\n", t.score);

    return 0;
}
