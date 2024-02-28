#define BOARD_HEIGHT 8
#include "tetris.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>

#define DIN_PIN PB2
#define CS_PIN PB3
#define CLK_PIN PB4

#define NUM_DEVICES 1

uint8_t buffer[2 * NUM_DEVICES];

// the opcodes for the MAX7221 and MAX7219
#define OP_NOOP 0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE 9
#define OP_INTENSITY 10
#define OP_SCANLIMIT 11
#define OP_SHUTDOWN 12
#define OP_DISPLAYTEST 15

struct led_controller {
        volatile uint8_t *ddr;
        volatile uint8_t *port;
        uint8_t din_pin;
        uint8_t cs_pin;
        uint8_t clk_pin;
        uint8_t num_devices;
        uint8_t *buffer;
};

void shift_out_msbfirst(volatile uint8_t *port, uint8_t din_pin,
                        uint8_t clk_pin, uint8_t data) {
    for (int i = 7; i >= 0; i--) {
        uint8_t mask = 1 << i;
        *port &= ~_BV(clk_pin);

        if (data & mask) {
            *port |= _BV(din_pin);
        } else {
            *port &= ~_BV(din_pin);
        }

        *port |= _BV(clk_pin);
    }
}

void led_controller_send_command(struct led_controller *controller,
                                 uint8_t addr, volatile uint8_t opcode,
                                 volatile uint8_t data) {
    // Create an array with the data to shift out
    int offset = addr * 2;
    int maxbytes = controller->num_devices * 2;

    for (int i = 0; i < maxbytes; i++) {
        controller->buffer[i] = 0;
    }

    // put our device data into the array
    controller->buffer[offset + 1] = opcode;
    controller->buffer[offset] = data;

    // enable the line
    *controller->port &= ~_BV(controller->cs_pin);

    // Now shift out the data
    for (int i = maxbytes - 1; i >= 0; i--) {
        shift_out_msbfirst(controller->port, controller->din_pin,
                           controller->clk_pin, controller->buffer[i]);
    }

    // latch the data onto the display
    *controller->port |= _BV(controller->cs_pin);
}

void led_controller_set_scan_limit(struct led_controller *controller,
                                   uint8_t addr, uint8_t limit) {
    if (addr < 0 || addr >= controller->num_devices) {
        return;
    }
    if (limit >= 0 && limit < 8) {
        led_controller_send_command(controller, addr, OP_SCANLIMIT, limit);
    }
}

void led_controller_set_intensity(struct led_controller *controller,
                                  uint8_t addr, uint8_t intensity) {
    if (addr < 0 || addr >= controller->num_devices) {
        return;
    }
    if (intensity >= 0 && intensity < 16) {
        led_controller_send_command(controller, addr, OP_INTENSITY, intensity);
    }
}

void led_controller_clear_display(struct led_controller *controller,
                                  uint8_t addr) {
    if (addr < 0 || addr >= controller->num_devices) {
        return;
    }

    for (uint8_t i = 0; i < 8; i++) {
        led_controller_send_command(controller, addr, i + 1, 0);
    }
}

void led_controller_shutdown(struct led_controller *controller, uint8_t addr,
                             uint8_t b) {
    if (addr < 0 || addr >= controller->num_devices) {
        return;
    }

    uint8_t value = b ? 0 : 1;

    led_controller_send_command(controller, addr, OP_SHUTDOWN, value);
}

void led_controller_init(struct led_controller *controller,
                         volatile uint8_t *ddr, volatile uint8_t *port,
                         uint8_t din_pin, uint8_t cs_pin, uint8_t clk_pin,
                         uint8_t num_devices, uint8_t *buffer) {
    // Set the controller struct
    controller->ddr = ddr;
    controller->port = port;
    controller->din_pin = din_pin;
    controller->cs_pin = cs_pin;
    controller->clk_pin = clk_pin;
    controller->num_devices = num_devices;
    controller->buffer = buffer;

    // Set the pins to output
    *controller->ddr |= _BV(controller->din_pin) | _BV(controller->cs_pin) |
                        _BV(controller->clk_pin);

    // Deselect all devices
    *controller->port |= _BV(controller->cs_pin);

    for (int i = 0; i < num_devices; i++) {
        led_controller_send_command(controller, i, OP_DISPLAYTEST, 0);
        // scanlimit is set to max on startup
        led_controller_set_scan_limit(controller, i, 7);
        // decode is done in source
        led_controller_send_command(controller, i, OP_DECODEMODE, 0);
        led_controller_clear_display(controller, i);
        // we go into shutdown-mode on startup
        led_controller_shutdown(controller, i, 1);
    }
}

void led_controller_set_row(struct led_controller *controller, uint8_t addr,
                            uint8_t row, uint8_t value) {
    if (addr < 0 || addr >= controller->num_devices) {
        return;
    }
    if (row < 0 || row > 7) {
        return;
    }
    led_controller_send_command(controller, addr, row + 1, value);
}

int tetris_rand_int(int max) { return rand() % max; }

int main() {
    int is_game_over = 0;

    struct tetris t;
    tetris_init(&t);

    struct led_controller controller;
    led_controller_init(&controller, &DDRB, &PORTB, DIN_PIN, CS_PIN, CLK_PIN,
                        NUM_DEVICES, buffer);

    led_controller_shutdown(&controller, 0, 0);
    led_controller_set_intensity(&controller, 0, 8);
    led_controller_clear_display(&controller, 0);

    is_game_over = tetris_spawn(&t);
    for (int i = 0; i < BOARD_BUFFER; i++) {
        led_controller_set_row(&controller, 0, i, t.board[i]);
    }

    while (1) {
        is_game_over = tetris_spawn(&t);
        for (int i = 0; i < BOARD_BUFFER; i++) {
            led_controller_set_row(&controller, 0, i, t.board[i]);
        }

        _delay_ms(1000);
    }

    return 0;
}
