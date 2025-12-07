#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "sound.h"

// -----------------------------------------------------------------------------
// Audio FX sound module control
//
// We use the Adafruit Audio FX (or similar) trigger pins T00?T05.
//
// Wiring (MCU -> Audio FX):
//   PD2  -> T00  (hello)
//   PD5  -> T01  (sleep)
//   PC1  -> T02  (thirsty)
//   PC2  -> T03  (light_off)
//   PC3  -> T04  (light_on)
//   PD4  -> T05  (stand_up)
//
// The trigger pins on the Audio FX board are active-low inputs.  Inactive
// state is HIGH; to play a track we pull the corresponding pin LOW for a
// short pulse, then release it back HIGH.
// -----------------------------------------------------------------------------

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    uint8_t           bit;
} sound_pin_t;

static const sound_pin_t sound_pins[SOUND_MAX] = {
    [SOUND_HELLO]     = { &DDRD, &PORTD, PD2 },
    [SOUND_SLEEP]     = { &DDRD, &PORTD, PD5 },
    [SOUND_THIRSTY]   = { &DDRD, &PORTD, PC1 },
    [SOUND_LIGHT_OFF] = { &DDRB, &PORTB, PC2 },
    [SOUND_LIGHT_ON]  = { &DDRB, &PORTB, PC3 },
    [SOUND_STAND_UP]  = { &DDRB, &PORTB, PD4 },
};

// Configure all trigger pins as outputs and set them HIGH (inactive)
void sound_init(void)
{
    for (uint8_t i = 0; i < SOUND_MAX; i++) {
        *sound_pins[i].ddr  |= (1 << sound_pins[i].bit);
        *sound_pins[i].port |= (1 << sound_pins[i].bit);
    }
}

static void sound_pulse(const sound_pin_t *p)
{
    // Make sure pin is an output and currently HIGH
    *p->ddr  |= (1 << p->bit);
    *p->port |= (1 << p->bit);
    _delay_ms(5);

    // Active-low pulse to trigger playback
    *p->port &= ~(1 << p->bit);
    _delay_ms(150);      // hold low long enough for the board to latch
    *p->port |= (1 << p->bit);
}

void sound_play(sound_id_t sound)
{
    if (sound >= SOUND_MAX) {
        return;
    }

    sound_pulse(&sound_pins[sound]);
}
