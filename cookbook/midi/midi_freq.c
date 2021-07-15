/*
 * XXX REVISIT: The floats in this module are single precision,
 * because as far as I know, that's the precision used by
 * a DAW.  But on modern FPUs (and why would you run a DAW
 * on anything but one?), double precision should be just
 * as fast.
 */
#include "midi_freq.h"
#include <math.h>

/* Our known reference: A-natural above Middle C */
static const float A_NOTE = 69.0f;
static const float A_FREQ = 440.0f;

/* log2f(1/440) */
static const float LOG_A_FREQ = -8.78135971352f;

/*
 * Helper to calculate the number of cents
 * between @frequequcy and our known reference.
 */
static inline float
cents_from_reference(float frequency)
{
        /*
         * logf() returns the log base e, not 10,
         * hence the ln(2) in the octave scalar.
         *
         * Also, log(f/440) == log(f*(1/440)) == log(f) + log(1/440).
         * I'm assuming addition is faster than multiplication
         * in an FPU... don't really know.
         *
         * XXX REVISIT: fast_log2 in libpbd is much quicker,
         * and accurate to three decimal places, which is
         * more than sufficient here.
         */
        return 1200.0f * (log2f(frequency) + LOG_A_FREQ);
}

/**
 * frequency_to_midi_note - Return nearest MIDI note
 *                      coresponding to @frequency
 * @frequency: Frequency to calculate the note of
 * @cents: Pointer to variable to store number of cents
 *      off of the return value where @frequency falls,
 *      or NULL if you don't need this value.  The result
 *      should be in the range of (-100:+100).
 *
 * Return: Closest MIDI note, 0-127.  If @frequency is higher
 *      than the nearest MIDI note, then @cents will store
 *      a negative number of cents.  Vice-versa if @frequency
 *      is lower than the nearest MIDI note.
 *
 *      If @frequency is out of range, then the result stored
 *      in @cents will be 0 and the return value will be either
 *      0 or 127, based on which boundary @frequency exceeds.`
 */
uint8_t
frequency_to_midi_note(float frequency, int *cents)
{
        /* One octave (in cents) time ln(2) */
        static const float HUNDREDTH = 1.0f / 100.0f;
        float inote, rem, c, note;

        c = cents_from_reference(frequency);

        note = A_NOTE + c * HUNDREDTH;
        if (note < 0.0f) {
                /* @frequency too low out of range */
                if (cents)
                        *cents = 0;
                return 0;
        }

        /* Calculate cents from new note */
        rem = modff(note, &inote);
        if (rem >= 0.5f) {
                rem -= 1.0f;
                inote += 1.0f;
        }

        if (inote > 127.0f) {
                /* @frequency too high out of range */
                if (cents)
                        *cents = 0;
                return 127;
        }

        if (cents) {
                rem *= 100.0f;
                if (rem < 0.0f)
                        rem -= 0.5;
                else
                        rem += 0.5;
                *cents = (int)rem;
        }
        return (int)inote & 0xffu;
}

static float
cents_to_frequency(float ref_freq, float cents)
{
        /* FIXME: I don't understand the x2 here */
        return ref_freq * powf(2.0f, cents / 1200.0f);
}

/**
 * midi_note_to_frequency - Get the frequency of a MIDI note
 * @midi_note: midi_note, 0-127
 *
 * Return: Frequency of @midi_note, in Hertz
 */
float
midi_note_to_frequency(uint8_t midi_note)
{
        return midi_note_and_cents_to_frequency(midi_note, 0.0f);
}

/**
 * midi_note_and_cents_to_frequency - Get the frequency of a
 *              MIDI note offset by @cents number of cents
 * @midi_note: midi_note, 0-127
 * @cents: Number of cents, typically -100 to 100, though it
 *      could be larger if the idea is you are going beyond
 *      a semitone.
 *
 * Return: Frequency corresponding to @note shifted by @cents
 */
float
midi_note_and_cents_to_frequency(uint8_t midi_note, int cents)
{
        cents += 100.0f * (float)(midi_note - A_NOTE);
        return cents_to_frequency(A_FREQ, cents);
}

/**
 * frequency_shift_cents - Get @freq shifted by @cents amount
 * @freq: Starting frequency
 * @cents: Number of cents to shift; a positive number to shift
 *      up and a negative number to shift down.
 *
 * Return: New frequency.
 */
float
frequency_shift_cents(float freq, int cents)
{
        return cents_to_frequency(freq, cents);
}

