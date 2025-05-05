#include "asteroids.h"
#include "player.h"
#include <errno.h>
#include <fcntl.h>
#include <fui/clock.h>
#include <fui/debugui.h>
#include <fui/events.h>
#include <fui/fbi.h>
#include <fui/fonts.h>
#include <fui/img/imgs.h>
#include <fui/input/keyboard.h>
#include <fui/input/mouse.h>
#include <fui/primitives.h>
#include <fui/sound/notes.h>
#include <fui/sound/sound.h>
#include <fui/tty.h>
#include <linux/input.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define M_PI 3.14159265358979323846

typedef struct scale {
    float notes[8];
} scale_t;

typedef struct chord {
    float *notes;
} chord_t;

typedef struct progression {
    chord_t *chords;
    int num_chords;
} progression_t;

void *play_melody_thread(void *arg) {
    (void)arg;
    scale_t CMajor = {.notes = {C4, D4, E4, F4, G4, A4, B4, C5}};
    scale_t CMinor = {.notes = {C4, D4, Ds4, F4, G4, Gs4, As4, C5}};
    progression_t CMajorChords = {.chords = NULL, .num_chords = 0};
    progression_t CMinorChords = {.chords = NULL, .num_chords = 0};
    CMajorChords.chords = (chord_t *)malloc(sizeof(chord_t) * 4);
    CMajorChords.num_chords = 4;
    CMajorChords.chords[0].notes = (float *)malloc(sizeof(float) * 3);
    CMajorChords.chords[0].notes[0] = C4;
    CMajorChords.chords[0].notes[1] = E4;
    CMajorChords.chords[0].notes[2] = G4;
    CMajorChords.chords[1].notes = (float *)malloc(sizeof(float) * 3);
    CMajorChords.chords[1].notes[0] = F4;
    CMajorChords.chords[1].notes[1] = A4;
    CMajorChords.chords[1].notes[2] = C5;
    CMajorChords.chords[2].notes = (float *)malloc(sizeof(float) * 3);
    CMajorChords.chords[2].notes[0] = G4;
    CMajorChords.chords[2].notes[1] = B4;
    CMajorChords.chords[2].notes[2] = D4;
    CMajorChords.chords[3].notes = (float *)malloc(sizeof(float) * 3);
    CMajorChords.chords[3].notes[0] = A4;
    CMajorChords.chords[3].notes[1] = C5;
    CMajorChords.chords[3].notes[2] = E4;
    CMinorChords.chords = (chord_t *)malloc(sizeof(chord_t) * 4);
    CMinorChords.num_chords = 4;
    CMinorChords.chords[0].notes = (float *)malloc(sizeof(float) * 3);
    CMinorChords.chords[0].notes[0] = C4;
    CMinorChords.chords[0].notes[1] = Ds4;
    CMinorChords.chords[0].notes[2] = G4;
    CMinorChords.chords[1].notes = (float *)malloc(sizeof(float) * 3);
    CMinorChords.chords[1].notes[0] = F4;
    CMinorChords.chords[1].notes[1] = Gs4;
    CMinorChords.chords[1].notes[2] = C5;
    CMinorChords.chords[2].notes = (float *)malloc(sizeof(float) * 3);
    CMinorChords.chords[2].notes[0] = G4;
    CMinorChords.chords[2].notes[1] = As4;
    CMinorChords.chords[2].notes[2] = D4;
    CMinorChords.chords[3].notes = (float *)malloc(sizeof(float) * 3);
    CMinorChords.chords[3].notes[0] = Gs4;
    CMinorChords.chords[3].notes[1] = C5;
    CMinorChords.chords[3].notes[2] = E4;

    while (true) {
        int random_index = rand() % 8;
        float note = CMajor.notes[random_index];
        play_sine_note(note, 0.25);
        random_index = rand() % 4;
        chord_t *chord = &CMajorChords.chords[random_index];
        play_sine_chord(chord->notes, 3, 0.25);
    }
    return NULL;
}

void *play_melody() {
    pthread_t thread;
    if (pthread_create(&thread, NULL, play_melody_thread, NULL) != 0) {
        fprintf(stderr, "Error creating melody thread\n");
        return NULL;
    }
}

int main() {
    srand(time(NULL));

    set_input_mode();

    framebuffer_t fb;
    load_framebuffer(&fb, DEFAULT_FB);
    if (fb.fd == -1) {
        reset_input_mode();
        fprintf(stderr, "Error loading framebuffer\n");
        return 1;
    }

    load_font("../../fonts/basis33.ttf");
    init_debugui("log.txt");
    init_fps();

    EventQueue event_queue;
    init_event_queue(&event_queue, EVENT_QUEUE_CAPACITY);
    init_keyboard();
    init_mouse(fb.width, fb.height);
    start_polling(&event_queue);

    init_sound();
    play_melody();

    layer_list_t *layer_list = NULL;
    create_layer_list(&layer_list);
    if (layer_list == NULL) {
        destroy_framebuffer(&fb);
        reset_input_mode();
        fprintf(stderr, "Error creating layer list\n");
        return 1;
    }
    add_layer(layer_list, 0, 0, fb.line_length_pixels, fb.height, 0.0f);
    layer_t *drawing_layer = &layer_list->layers[0];

    clear_framebuffer(&fb, 0xFF000000);

    asteroids_t *asteroid_list = (asteroids_t *)malloc(sizeof(asteroids_t));
    int max_asteroid_count = 200;
    int min_vtx = 4;
    int max_vtx = 8;
    float min_speed = 20.0f;
    float max_speed = 250.0f;
    float min_x = 0.0f;
    float max_x = (float)fb.width;
    float dx = 200.0f;
    float min_y = 0.0f;
    float max_y = (float)fb.height;
    float dy = 200.0f;
    init_asteroid_list(asteroid_list, max_asteroid_count, min_vtx, max_vtx, min_speed, max_speed, min_x, max_x, dx,
                       min_y, max_y, dy);

    player_t *player = (player_t *)malloc(sizeof(player_t));
    init_player(player, fb.width / 2.0f, fb.height / 2.0f);
    float max_player_speed = 600.0f;
    uint64_t last_time = get_time_us();
    uint64_t current_time = get_time_us();
    uint64_t delta_time = 0;
    float fixed_dt = 0.0016f;
    float accumulator = 0.0f;

    while (1) {
        current_time = get_time_us();
        delta_time = current_time - last_time;
        last_time = current_time;
        accumulator += (float)delta_time / 1000000.0f;

        process_event_queue(&event_queue);

        if (global_keyboard_state.key_state[KEY_UP]) {
            if (powf(player->vx, 2) + powf(player->vy, 2) < powf(max_player_speed, 2)) {
                player->vx -= sinf(player->orientation) * 10.0f;
                player->vy += cosf(player->orientation) * 10.0f;
            }
        }
        if (global_keyboard_state.key_state[KEY_LEFT]) {
            player->orientation -= 0.025f;
            if (player->orientation < 0.0f) {
                player->orientation += 2.0f * M_PI;
            }
        }
        if (global_keyboard_state.key_state[KEY_RIGHT]) {
            player->orientation += 0.025f;
            if (player->orientation > 2.0f * M_PI) {
                player->orientation -= 2.0f * M_PI;
            }
        }
        if (global_keyboard_state.key_state[KEY_SPACE]) {
            shoot(player);
        }

        while (accumulator >= fixed_dt) {
            if (rand() % 100 == 0) {
                create_asteroid(asteroid_list);
            }
            update_player(player, fixed_dt, 0.0f, (float)fb.width, 0.0f, (float)fb.height);
            update_bullets(bullet_list, fixed_dt, 0.0f, (float)fb.width, 0.0f, (float)fb.height);
            score += update_asteroids(asteroid_list, fixed_dt, bullet_list);
            accumulator -= fixed_dt;
        }

        if (collision_detection(asteroid_list, player)) {
            pdebugui("Game Over\n");
            break;
        }

        if (global_mouse_state.middle) {
            screenshot("screenshot.png", &fb);
            pdebugui("Screenshot taken");
        }

        gray_clear_framebuffer(&fb, 0x00);

        clear_layer(layer_list, 0);

        draw_asteroids(drawing_layer, asteroid_list);
        draw_player(drawing_layer, player);
        draw_bullets(drawing_layer, bullet_list);

        const char *score_str = (char *)malloc(32);
        snprintf(score_str, 32, "Score: %d", score);
        draw_text(drawing_layer, score_str, 10, 30, 50.0f, 0xFFFFFFFF);
        update_fps();
        render_debugui(&layer_list->layers[0], 10, 10, 20.0f);
        render_fps(&layer_list->layers[0], 10, layer_list->layers[0].height - 20, 20.0f);

        if (global_keyboard_state.key_state[KEY_ESC]) {
            break;
        }

        draw_all_layers(layer_list, &fb, false, false);
        render_framebuffer(&fb);
    }

    destroy_asteroid_list(asteroid_list);

    cleanup_debugui();

    destroy_layer_list(&layer_list);
    destroy_framebuffer(&fb);

    cleanup_polling();
    destroy_event_queue(&event_queue);
    cleanup_mouse();
    cleanup_keyboard();

    reset_input_mode();

    return 0;
}
