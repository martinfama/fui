#ifndef __ASTEROIDS_H_
#define __ASTEROIDS_H__

#include "player.h"
#include <fui/fbi.h>

typedef struct asteroid {
  float x, y;
  float vx, vy;
  int vtx_count;
  float *vtx_x;
  float *vtx_y;
} asteroid_t;

typedef struct asteroids {
  asteroid_t *asteroids;
  int count;
  int max_count;
  float speed_range[2];
  int vtx_range[2];
  float min_x, max_x, dx;
  float min_y, max_y, dy;
} asteroids_t;

void init_asteroid_list(asteroids_t *asteroid_list, int max_count, int min_vtx,
                        int max_vtx, float min_speed, float max_speed,
                        float min_x, float max_x, float dx, float min_y,
                        float max_y, float dy);
void destroy_asteroid_list(asteroids_t *asteroid_list);
void create_asteroid(asteroids_t *asteroid_list);
void destroy_asteroid(asteroids_t *asteroid_list, int index);
int update_asteroids(asteroids_t *asteroid_list, float dt,
                     bullet_list_t *bullet_list);
void draw_asteroids(layer_t *layer, asteroids_t *asteroid_list);
bool point_polygon_intersection(float x, float y, asteroid_t *a);
bool collision_detection(asteroids_t *asteroid_list, player_t *player);

#endif // __ASTEROIDS_H__