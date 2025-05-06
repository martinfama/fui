#include "sound.h"
#include "notes.h"
#include <alsa/asoundlib.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define CHANNELS 1
#define FORMAT SND_PCM_FORMAT_S16_LE
#define M_PI 3.14159265358979323846
#define FADE_DURATION 200

static snd_pcm_t *handle;
static snd_pcm_hw_params_t *params;
static int err;

void init_sound() {
  err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (err < 0) {
    fprintf(stderr, "Failed to open PCM device: %s\n", snd_strerror(err));
    return;
  }

  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(handle, params);
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(handle, params, FORMAT);
  snd_pcm_hw_params_set_channels(handle, params, CHANNELS);
  snd_pcm_hw_params_set_rate(handle, params, SAMPLE_RATE, 0);

  snd_pcm_hw_params(handle, params);
}

void close_sound() {
  if (handle) {
    snd_pcm_close(handle);
    handle = NULL;
  }
}

void play_buffer(int16_t *buffer, int size) {
  snd_pcm_writei(handle, buffer, size);
}

void play_sine_note(float frequency, float duration) {
  int16_t buffer[SAMPLE_RATE * CHANNELS];
  for (int i = 0; i < SAMPLE_RATE * duration; i++) {
    float t = (float)i / SAMPLE_RATE;
    if (i < FADE_DURATION) {
      buffer[i] = (int16_t)(sin(2 * M_PI * frequency * t) * 32767 * (float)i /
                            FADE_DURATION);
    } else if (i > SAMPLE_RATE * duration - FADE_DURATION) {
      buffer[i] =
          (int16_t)(sin(2 * M_PI * frequency * t) * 32767 *
                    (float)(SAMPLE_RATE * duration - i) / FADE_DURATION);
    } else {
      buffer[i] = (int16_t)(sin(2 * M_PI * frequency * t) * 32767);
    }
  }
  play_buffer(buffer, SAMPLE_RATE * duration);
}

void play_sine_chord(float *frequencies, int num_frequencies, float duration) {
  int16_t buffer[SAMPLE_RATE * CHANNELS];
  for (int i = 0; i < SAMPLE_RATE * duration; i++) {
    float t = (float)i / SAMPLE_RATE;
    buffer[i] = 0;
    for (int j = 0; j < num_frequencies; j++) {
      float phase_offset = (float)j / num_frequencies;
      if (i < FADE_DURATION) {
        buffer[i] +=
            (int16_t)(sin(2 * M_PI * frequencies[j] * t + phase_offset) *
                      32767 * (float)i / FADE_DURATION / num_frequencies);
      } else if (i > SAMPLE_RATE * duration - FADE_DURATION) {
        buffer[i] +=
            (int16_t)(sin(2 * M_PI * frequencies[j] * t + phase_offset) *
                      32767 * (float)(SAMPLE_RATE * duration - i) /
                      FADE_DURATION / num_frequencies);
      } else {
        buffer[i] +=
            (int16_t)(sin(2 * M_PI * frequencies[j] * t + phase_offset) *
                      32767 / num_frequencies);
      }
    }
  }
  play_buffer(buffer, SAMPLE_RATE * duration);
}

// triangle wave is: x(t) = 2 * | t/p - floor(t/p + 0.5) |
void play_triangle_note(float frequency, float duration) {
  int16_t buffer[SAMPLE_RATE * CHANNELS];
  float period = 1.0f / frequency;
  for (int i = 0; i < SAMPLE_RATE * duration; i++) {
    float t = (float)i / SAMPLE_RATE;
    float triangle_wave = 2.0f * fabsf(fmodf(t / period, 1.0f) - 0.5f) - 1.0f;
    if (i < FADE_DURATION) {
      buffer[i] = (int16_t)(triangle_wave * 32767 * (float)i / FADE_DURATION);
    } else if (i > SAMPLE_RATE * duration - FADE_DURATION) {
      buffer[i] =
          (int16_t)(triangle_wave * 32767 *
                    (float)(SAMPLE_RATE * duration - i) / FADE_DURATION);
    } else {
      buffer[i] = (int16_t)(triangle_wave * 32767);
    }
  }
  play_buffer(buffer, SAMPLE_RATE * duration);
}