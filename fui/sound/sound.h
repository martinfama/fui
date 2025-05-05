#ifndef __SOUND_H__
#define __SOUND_H__

#include <alsa/asoundlib.h>

void init_sound();
void close_sound();
void play_buffer(int16_t *buffer, int size);
void play_sine_note(float frequency, float duration);
void play_sine_chord(float *frequencies, int num_frequencies, float duration);
void play_triangle_note(float frequency, float duration);

#endif // __SOUND_H__