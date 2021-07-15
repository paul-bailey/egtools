#include "midi.h"

/*
 * Parse a single MIDI message.
 */
int
parse_midi_pseudocode(const uint8_t *data, size_t size)
{
        unsigned int note;
        switch (size) {
        case 1:
                switch (data[0]) {
                case 0xff:
                case 0xfc:
                        /* Sound off, release all notes */
                        all_notes_off();
                        break;
                case 0xf8:
                        /* Handle timing message or ignore */
                        break;
                default:
                        return -1;
                }
                break;
        case 3:
                switch (data[0] & 0xf0) {
                case MIDI_NOTE_ON:
                        note = data[1];
                        /* or just note_on_off(note, data[2]) */
                        if (data[2] != 0)
                                note_on(note, data[2]);
                        else
                                note_off(note);
                        break;
                case MIDI_NOTE_OFF:
                        note = data[1];
                        note_off(note);
                        break;
                case MIDI_CONTROL_CHANGE:
                        switch (data[1]) {
                        case MIDI_DAMPER_ONOFF:
                                sustain_on_off(data[2] >= 64);
                                break;
                        case MIDI_MODWHEEL_MSB:
                                set_mod_wheel(data[2]);
                                break;
                        case MIDI_ALL_SOUND_OFF:
                        case MIDI_ALL_NOTES_OFF:
                                all_notes_off();
                                break;
                        default:
                                return -1;
                        }
                case MIDI_PITCH_WHEEL:
                        pitch_bend(data[1]
                                   | ((unsigned int)data[2] << 7));
                        break;
                default:
                        return -1;
                }
                break;
        default:
                return -1;
        }
        return 0;
}
