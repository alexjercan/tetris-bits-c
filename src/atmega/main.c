#define NUM_DEVICES 1

#include <string.h>
#define BOARD_HEIGHT (8 * NUM_DEVICES)
#define TETRIS_IMPLEMENTATION
#include "tetris.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#define LC_IMPLEMENTATION
#include "lc.h"

#define DIN_PIN PB2
#define CS_PIN PB3
#define CLK_PIN PB4

uint8_t buffer[2 * NUM_DEVICES];

int tetris_rand_int(int max) { return rand() % max; }

struct tetris t;
struct tetris tmp;
struct led_controller controller;

int main() {
    int is_game_over = 0;

    lc_init(&controller, &DDRB, &PORTB, DIN_PIN, CS_PIN, CLK_PIN, NUM_DEVICES,
            buffer);

    lc_shutdown(&controller, 0, 0);
    lc_set_intensity(&controller, 0, 8);
    lc_clear_display(&controller, 0);

    tetris_init(&t);

    while (1) {
        tmp = t;

        enum move m = MOVE_NONE;
        if (tetris_rand_int(2) == 0) {
            m = MOVE_LEFT;
        } else {
            m = MOVE_RIGHT;
        }
        tetris_tick(&t, m);

        if (memcmp(tmp.board, t.board, (BOARD_HEIGHT + BOARD_BUFFER)) == 0) {
            is_game_over = tetris_spawn(&t);
        }

        for (int i = 0; i < BOARD_HEIGHT; i++) {
            lc_set_row(&controller, 0, i, t.board[i + BOARD_BUFFER]);
        }

        if (is_game_over) {
            tetris_init(&t);
            lc_clear_display(&controller, 0);
            is_game_over = 0;
        }

        _delay_ms(1000);
    }

    return 0;
}
