#include "asteroids.h"
#include "player.h"
#include <errno.h>
#include <fcntl.h>
#include <fui/debugui.h>
#include <fui/fbi.h>
#include <fui/fonts.h>
#include <fui/primitives.h>
#include <linux/input.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define M_PI 3.14159265358979323846

void init_asteroid_list(asteroids_t *asteroid_list, int max_count, int min_vtx,
                        int max_vtx, float min_speed, float max_speed,
                        float min_x, float max_x, float dx, float min_y,
                        float max_y, float dy) {
  asteroid_list->asteroids =
      (asteroid_t *)malloc(max_count * sizeof(asteroid_t));
  // set vtx to NULL, will malloc on the fly
  for (int i = 0; i < max_count; i++) {
    asteroid_list->asteroids[i].vtx_x = NULL;
    asteroid_list->asteroids[i].vtx_y = NULL;
  }
  asteroid_list->count = 0;
  asteroid_list->max_count = max_count;
  asteroid_list->speed_range[0] = min_speed;
  asteroid_list->speed_range[1] = max_speed;
  asteroid_list->vtx_range[0] = min_vtx;
  asteroid_list->vtx_range[1] = max_vtx;
  asteroid_list->min_x = min_x;
  asteroid_list->max_x = max_x;
  asteroid_list->dx = dx;
  asteroid_list->min_y = min_y;
  asteroid_list->max_y = max_y;
  asteroid_list->dy = dy;
}

static float ufloat(float min, float max) {
  float scale = rand() / (float)RAND_MAX; // [0, 1.0]
  return min + scale * (max - min);       // [min, max]
}

void create_asteroid(asteroids_t *asteroid_list) {
  if (asteroid_list->count >= asteroid_list->max_count) {
    return;
  }
  asteroid_t *a = &asteroid_list->asteroids[asteroid_list->count++];
  a->vtx_count =
      rand() % (asteroid_list->vtx_range[1] - asteroid_list->vtx_range[0]) +
      asteroid_list->vtx_range[0];
  a->vtx_x = (float *)malloc(a->vtx_count * sizeof(float));
  a->vtx_y = (float *)malloc(a->vtx_count * sizeof(float));
  if (a->vtx_x == NULL || a->vtx_y == NULL) {
    perror("Error allocating memory for asteroid vertices");
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < a->vtx_count; i++) {
    float angle = (float)i / (float)a->vtx_count * 2.0f * M_PI;
    float distance = (float)(rand() % 30 + 15);
    a->vtx_x[i] = cos(angle) * distance;
    a->vtx_y[i] = sin(angle) * distance;
  }
  float _dx = rand() % (int)asteroid_list->dx + 1;
  float _dy = rand() % (int)asteroid_list->dy + 1;
  a->x = (rand() % 2 == 0) ? asteroid_list->min_x - _dx
                           : asteroid_list->max_x + _dx;
  a->y = (rand() % 2 == 0) ? asteroid_list->min_y - _dy
                           : asteroid_list->max_y + _dy;

  float speed = (float)(rand() % (int)(asteroid_list->speed_range[1] -
                                       asteroid_list->speed_range[0])) +
                asteroid_list->speed_range[0];
  // calculate angle range from asteroid position to the four corners of the
  // screen, and choose a random angle in that range
  float angle_tl = atan2((float)(asteroid_list->min_y - a->y),
                         (float)(asteroid_list->min_x - a->x));
  float angle_tr = atan2((float)(asteroid_list->min_y - a->y),
                         (float)(asteroid_list->max_x - a->x));
  float angle_bl = atan2((float)(asteroid_list->max_y - a->y),
                         (float)(asteroid_list->min_x - a->x));
  float angle_br = atan2((float)(asteroid_list->max_y - a->y),
                         (float)(asteroid_list->max_x - a->x));
  float min_angle = fminf(fminf(angle_tl, angle_tr), fminf(angle_bl, angle_br));
  float max_angle = fmaxf(fmaxf(angle_tl, angle_tr), fmaxf(angle_bl, angle_br));
  float angle = ufloat(min_angle, max_angle);
  a->vx = cos(angle) * speed;
  a->vy = sin(angle) * speed;
}

void destroy_asteroid_list(asteroids_t *asteroid_list) {
  for (int i = 0; i < asteroid_list->count; i++) {
    free(asteroid_list->asteroids[i].vtx_x);
    free(asteroid_list->asteroids[i].vtx_y);
  }
  free(asteroid_list->asteroids);
  asteroid_list->count = 0;
  asteroid_list->max_count = 0;
  asteroid_list->speed_range[0] = 0.0f;
  asteroid_list->speed_range[1] = 0.0f;
  asteroid_list->vtx_range[0] = 0;
  asteroid_list->vtx_range[1] = 0;
  free(asteroid_list);
  asteroid_list = NULL;
}

int update_asteroids(asteroids_t *asteroid_list, float dt,
                     bullet_list_t *bullet_list) {
  int score = 0;
  for (int i = 0; i < asteroid_list->count; i++) {
    asteroid_t *a = &asteroid_list->asteroids[i];
    a->x += a->vx * dt;
    a->y += a->vy * dt;
    if (a->x < asteroid_list->min_x - asteroid_list->dx ||
        a->y < asteroid_list->min_y - asteroid_list->dy ||
        a->x > asteroid_list->max_x + asteroid_list->dx ||
        a->y > asteroid_list->max_y + asteroid_list->dy) {
      destroy_asteroid(asteroid_list, i);
      if (i == asteroid_list->count || asteroid_list->count == 0) {
        pdebugui("No more asteroids to check\n");
        break;
      }
      i--;
    }
    for (int j = 0; j < bullet_list->count; j++) {
      bullet_t *b = &bullet_list->bullets[j];
      if (point_polygon_intersection(b->x, b->y, a)) {
        destroy_asteroid(asteroid_list, i);
        score++;
        destroy_bullet(bullet_list, j);
        if (j == bullet_list->count || bullet_list->count == 0) {
          break; // no more bullets to check
        }
        if (i == asteroid_list->count || asteroid_list->count == 0) {
          break; // no more asteroids to check
        }
        i--;
        break;
      }
    }
  }
  return score;
}

void destroy_asteroid(asteroids_t *asteroid_list, int index) {
  if (index < 0 || index >= asteroid_list->count) {
    return;
  }
  asteroid_t *a = &asteroid_list->asteroids[index];
  free(a->vtx_x);
  free(a->vtx_y);
  a->vtx_x = NULL;
  a->vtx_y = NULL;

  for (int j = index; j < asteroid_list->count - 1; j++) {
    asteroid_list->asteroids[j] = asteroid_list->asteroids[j + 1];
    asteroid_list->asteroids[j].vtx_x = asteroid_list->asteroids[j + 1].vtx_x;
    asteroid_list->asteroids[j].vtx_y = asteroid_list->asteroids[j + 1].vtx_y;
  }

  asteroid_list->count--;
}

void draw_asteroids(layer_t *layer, asteroids_t *asteroid_list) {
  for (int k = 0; k < asteroid_list->count; k++) {
    asteroid_t *a = &asteroid_list->asteroids[k];
    for (int i = 0; i < a->vtx_count; i++) {
      int x1 = a->x + a->vtx_x[i];
      int y1 = a->y + a->vtx_y[i];
      int x2 = a->x + a->vtx_x[(i + 1) % a->vtx_count];
      int y2 = a->y + a->vtx_y[(i + 1) % a->vtx_count];
      draw_line(layer, x1, y1, x2, y2, 0xFFFFFFFF);
    }
  }
}

bool point_polygon_intersection(float x, float y, asteroid_t *a) {
  // Ray-casting algorithm to check if point is inside polygon
  bool inside = false;
  for (int i = 0, j = a->vtx_count - 1; i < a->vtx_count; j = i++) {
    float xi = a->x + a->vtx_x[i];
    float yi = a->y + a->vtx_y[i];
    float xj = a->x + a->vtx_x[j];
    float yj = a->y + a->vtx_y[j];

    if ((yi > y) != (yj > y) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
      inside = !inside;
    }
  }
  return inside;
}

bool collision_detection(asteroids_t *asteroid_list, player_t *player) {
  for (int i = 0; i < asteroid_list->count; i++) {
    asteroid_t *a = &asteroid_list->asteroids[i];
    for (int j = 0; j < a->vtx_count; j++) {
      float rotated_x = cosf(player->orientation) * a->vtx_x[j] -
                        sinf(player->orientation) * a->vtx_y[j];
      float rotated_y = sinf(player->orientation) * a->vtx_x[j] +
                        cosf(player->orientation) * a->vtx_y[j];
      if (point_polygon_intersection(player->x, player->y, a)) {
        return true;
      }
    }
  }
  return false;
}