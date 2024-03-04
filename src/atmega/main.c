#define TARGET_FPS 10
#define DELAY_MS (1000.0 / TARGET_FPS)
#define NUM_DEVICES 2

#include <string.h>
#define BOARD_HEIGHT (8 * NUM_DEVICES)
#define TETRIS_IMPLEMENTATION
#include "tetris.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#define LC_IMPLEMENTATION
#include "lc.h"

#define RBUTTON_PIN PB0
#define LBUTTON_PIN PB1
#define DIN_PIN PB2
#define CS_PIN PB3
#define CLK_PIN PB4

uint8_t buffer[2 * NUM_DEVICES];

int tetris_rand_int(int max) { return rand() % max; }

volatile int input = 0;
struct tetris t;
struct tetris tmp;
struct led_controller controller;

ISR(INT0_vect) { input = -1; }
ISR(INT1_vect) { input = 1; }

int main() {
    int is_game_over = 0;

    // Led matrix setup
    lc_init(&controller, &DDRB, &PORTB, DIN_PIN, CS_PIN, CLK_PIN, NUM_DEVICES,
            buffer);

    for (uint8_t i = 0; i < NUM_DEVICES; i++) {
        lc_shutdown(&controller, i, 0);
        lc_set_intensity(&controller, i, 4);
        lc_clear_display(&controller, i);
    }

    // Tetris setup
    tetris_init(&t);

    // Input setup
    EICRA |= _BV(ISC00) | _BV(ISC01) | _BV(ISC10) | _BV(ISC11);
    EIMSK |= _BV(INT0) | _BV(INT1);

    sei();

    while (1) {
        tmp = t;

        enum move m = MOVE_NONE;
        if (input > 0) {
            m = MOVE_RIGHT;
        } else if (input < 0) {
            m = MOVE_LEFT;
        }
        input = 0;

        tetris_tick(&t, m);

        if (memcmp(tmp.board, t.board, (BOARD_HEIGHT + BOARD_BUFFER)) == 0) {
            is_game_over = tetris_spawn(&t);
        }

        for (uint8_t i = 0; i < BOARD_HEIGHT; i++) {
            lc_set_row(&controller, i / 8, i % 8, t.board[i + BOARD_BUFFER]);
        }

        if (is_game_over) {
            tetris_init(&t);
            lc_clear_display(&controller, 0);
            is_game_over = 0;
        }

        _delay_ms(DELAY_MS);
    }

    return 0;
}
