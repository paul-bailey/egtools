/*
 * Some common definitions and useful info re: MIDI
 * This is not a complete library.
 * It's just some cookbook example-ey stuff.
 *
 * The functions in midi_freq.c which convert between
 * MIDI notes, cents, and frequency are complete enough, though.
 *
 * Paul Bailey, 2019
 */
#ifndef MIDI_H
#define MIDI_H

/* Manufacturer's MIDI ID numbers */
enum midi_manufacturer_t {
        MIDI_SEQUENTIAL = 0x01,
        MIDI_BIG_BRIAR  = 0x02,
        MIDI_OCTAVE     = 0x03,
        MIDI_MOOG       = 0x04, /* <3 <3 <3 */
        MIDI_PASSPORT   = 0x05,
        MIDI_LEXICON    = 0x06,
        MIDI_TEMPI      = 0x20,
        MIDI_SIEL       = 0x21,
        MIDI_KAWAI      = 0x41,
        MIDI_ROLAN      = 0x42,
        MIDI_KORG       = 0x42,
        MIDI_YAMAHA     = 0x43,
};

enum midi_command_t {
        MIDI_NOTE_OFF           = 0x80,
        MIDI_NOTE_ON            = 0x90,
        MIDI_POLY_AFTERTOUCH    = 0xA0,
        MIDI_CONTROL_CHANGE     = 0xB0,
        MIDI_PROGRAM_CHANGE     = 0xC0,
        MIDI_CHAN_AFTERTOUCH    = 0xD0,
        MIDI_PITCH_WHEEL        = 0xE0,
        MIDI_SYSEX              = 0xF0,
        MIDI_DELAY_PACKET       = 1111,
};

/* The most interesting CC controls (command == MIDI_CONTROL_CHANGE) */
enum midi_cc_t {
        MIDI_BANK_SEL_MSB       = 0,  /* 0-127 */
        MIDI_MOD_WHEEL_MSB      = 1,  /* 0-127 */
        MIDI_BREATH_MSB         = 2,  /* 0-127 */
        MIDI_PORTAMENTO_MSB     = 5,  /* 0-127 */
        MIDI_PAN_MSB            = 10, /* 0-127 */
        MIDI_EXPRESSION_MSB     = 11, /* 0-127 */
        MIDI_FX1_MSB            = 12, /* 0-127 */
        MIDI_FX2_MSB            = 13, /* 0-127 */
        MIDI_BANK_SEL_LSB       = 32, /* 0-127 */
        MIDI_MOD_WHEEL_LSB      = 33, /* 0-127 */
        MIDI_BREATH_LSB         = 34, /* 0-127 */
        MIDI_PORTAMENT0_LSB     = 37, /* 0-127 */
        MIDI_PAN_LSB            = 42, /* 0-127 */
        MIDI_EXPRESSION_LSB     = 43, /* 0-127 */
        MIDI_DAMPER_ONOFF       = 64, /* <= 63 off, >= 64 on */
        MIDI_PORTAMENTO_ONOFF   = 65, /* <= 63 off, >= 64 on */
        MIDI_SOSTENUTO_ONOFF    = 66, /* <= 63 off, >= 64 on */
        MIDI_SOFTPEDAL_ONOFF    = 67, /* <= 63 off, >= 64 on */
        MIDI_LEGATO_FOOTSWITCH  = 68, /* <= 63 normal, >= 64 legato */

        /* Channel-mode CC */
        MIDI_ALL_SOUND_OFF      = 120, /* 0 */
        MIDI_RESET_CONTROLLERS  = 121, /* 0 */
        MIDI_LOCAL_ONOFF        = 122, /* 0 off, 127 on */
        MIDI_ALL_NOTES_OFF      = 123, /* 0 */
        MIDI_OMNI_MODE_OFF      = 124, /* 0 */
        MIDI_OMNI_MODE_ON       = 125, /* 0 */
        MIDI_MONO_ON            = 126, /* # of channels or 0, see spec */
        MIDI_POLY_ON            = 127, /* 0 */
};

#include <stdint.h>

extern uint8_t frequency_to_midi_note(float frequency, int *cents);
extern float midi_note_to_frequency(uint8_t midi_note);
extern float midi_note_and_cents_to_frequency(uint8_t midi_note, int cents);
extern float frequency_shift_cents(float freq, int cents);

#endif /* MIDI_H */
