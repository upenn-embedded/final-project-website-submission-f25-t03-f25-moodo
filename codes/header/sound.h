#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

// Logical IDs for the sound effects used in the project.
// These map to files on the Audio FX board:
//   SOUND_HELLO     -> T00.wav  ("hello")
//   SOUND_SLEEP     -> T01.wav  ("sleep")
//   SOUND_THIRSTY   -> T02.wav  ("thirsty")
//   SOUND_LIGHT_OFF -> T03.wav  ("light_off")
//   SOUND_LIGHT_ON  -> T04.wav  ("light_on")
//   SOUND_STAND_UP  -> T05.wav  ("stand_up")

typedef enum {
    SOUND_HELLO = 0,
    SOUND_SLEEP,
    SOUND_THIRSTY,
    SOUND_LIGHT_OFF,
    SOUND_LIGHT_ON,
    SOUND_STAND_UP,
    SOUND_MAX
} sound_id_t;

// Configure GPIO pins used to trigger the sound board.
void sound_init(void);

// Trigger playback of one of the sound files.
// This function generates a short active-low pulse on the corresponding pin.
void sound_play(sound_id_t sound);

#endif // SOUND_H
