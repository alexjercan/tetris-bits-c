#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

int tetris_rand_int(int max) {
    return rand() % max;
}

int main() {
    srand(42);

    DDRB = DDRB | _BV(DDB5);

    while (1) {
        PORTB = PORTB | _BV(PORTB5);

        _delay_ms(1000);

        PORTB = PORTB & ~_BV(PORTB5);

        _delay_ms(1000);
    }

    return 0;
}
