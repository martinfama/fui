#include <fui/debugui.h>
#include <fui/events.h>
#include <fui/fbi.h>
#include <fui/fonts.h>
#include <fui/input/keyboard.h>
#include <fui/input/mouse.h>
#include <fui/primitives.h>
#include <fui/tty.h>
#include <math.h>
#include <time.h>

#define G 6.67430e-02 // Gravitational constant
#define M_PI 3.14159265358979323846

struct body {
  double x;
  double y;
  double vx;
  double vy;
  double r;
  double mass;
  uint32_t color;
};

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

  layer_list_t *layer_list = NULL;
  create_layer_list(&layer_list);
  add_layer(layer_list, 0, 0, fb.width, fb.height, 0.0f);

  load_font("../fonts/basis33.ttf");

  init_debugui("debugui.log");
  init_fps();

  EventQueue event_queue;
  init_event_queue(&event_queue, EVENT_QUEUE_CAPACITY);
  init_keyboard();
  init_mouse(fb.width, fb.height);
  start_polling(&event_queue);

  int32_t num_bodies = 600;
  struct body *bodies = malloc(num_bodies * sizeof(struct body));
  for (int32_t i = 0; i < num_bodies; i++) {
    // set range to the middle 200x200 of the screen
    bodies[i].x = rand() % (fb.width - 400) + 200;
    bodies[i].y = rand() % (fb.height - 400) + 200;
    bodies[i].r = rand() % 10 + 5;
    bodies[i].mass = bodies[i].r * bodies[i].r * M_PI;
    bodies[i].vx = 0;
    bodies[i].vy = 0;
    double min_mass = 5 * 5 * M_PI;
    double max_mass = 14 * 14 * M_PI;
    double intensity = (bodies[i].mass - min_mass) / (max_mass - min_mass);
    intensity = intensity * 0.8 + 0.2;
    uint8_t alpha = (uint8_t)(intensity * 255);
    uint8_t red = 0;
    uint8_t green = 255;
    uint8_t blue = 0;
    bodies[i].color = (alpha << 24) | (red << 16) | (green << 8) | blue;
  }

  struct body massive_body;
  massive_body.x = fb.width / 2;
  massive_body.y = fb.height / 2;
  massive_body.mass = 100000;
  massive_body.r = sqrt(massive_body.mass / 3.14);
  massive_body.vx = 0;
  massive_body.vy = 0;
  massive_body.color = 0xFFFF0000;

  while (1) {
    process_event_queue(&event_queue);

    gray_clear_framebuffer(&fb, 0x00);

    clear_all_layers(layer_list);

    massive_body.x = global_mouse_state.x;
    massive_body.y = global_mouse_state.y;
    if (global_mouse_state.scroll != 0) {
      massive_body.mass *= pow(1.1, global_mouse_state.scroll);
      massive_body.mass = massive_body.mass < 10000     ? 10000
                          : massive_body.mass > 1000000 ? 1000000
                                                        : massive_body.mass;
      massive_body.r = sqrt(massive_body.mass / 3.14);
      global_mouse_state.scroll = 0;
    }

    for (int32_t i = 0; i < num_bodies; i++) {
      for (int32_t j = i + 1; j < num_bodies; j++) {
        double dx = bodies[j].x - bodies[i].x;
        double dy = bodies[j].y - bodies[i].y;
        double dist = sqrt(dx * dx + dy * dy);
        if (dist > 5) {
          double force = G * (bodies[i].mass * bodies[j].mass) / (dist * dist);
          double ax = force * dx / dist;
          double ay = force * dy / dist;

          bodies[i].vx += ax / bodies[i].mass;
          bodies[i].vy += ay / bodies[i].mass;
          bodies[j].vx -= ax / bodies[j].mass;
          bodies[j].vy -= ay / bodies[j].mass;
        }
      }

      double dx = massive_body.x - bodies[i].x;
      double dy = massive_body.y - bodies[i].y;
      double dist = sqrt(dx * dx + dy * dy);
      if (dist > 20) {
        double force = G * (massive_body.mass * bodies[i].mass) / (dist * dist);
        double ax = force * dx / dist / bodies[i].mass;
        double ay = force * dy / dist / bodies[i].mass;
        bodies[i].vx += ax;
        bodies[i].vy += ay;
      }
      if (dist > 500) {
        double force = G * (massive_body.mass * bodies[i].mass) / 100;
        double ax = force * dx / dist / bodies[i].mass;
        double ay = force * dy / dist / bodies[i].mass;
        bodies[i].vx += ax;
        bodies[i].vy += ay;
      }
    }

    for (int32_t i = 0; i < num_bodies; i++) {
      bodies[i].vx *= 0.7;
      bodies[i].vy *= 0.7;
      bodies[i].x += bodies[i].vx;
      bodies[i].y += bodies[i].vy;

      draw_rectangle(&layer_list->layers[0], (int32_t)bodies[i].x - 1,
                     (int32_t)bodies[i].y - 1, 3, 3, bodies[i].color, true);
    }

    draw_circle(&layer_list->layers[0], (int32_t)massive_body.x,
                (int32_t)massive_body.y, (int32_t)massive_body.r * 0.1f,
                massive_body.color, false);

    update_fps();
    render_fps(&layer_list->layers[0], 10, fb.height - 30, 20);
    render_debugui(&layer_list->layers[0], 10, 30, 20);

    draw_all_layers(layer_list, &fb, false, false);
    render_framebuffer(&fb);

    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n > 0) {
      if (c == 27) {
        break;
      }
    }
  }
  free(bodies);
  cleanup_debugui();

  destroy_layer_list(&layer_list);
  destroy_framebuffer(&fb);

  cleanup_polling();
  destroy_event_queue(&event_queue);
  cleanup_mouse();
  reset_input_mode();
  return 0;
}