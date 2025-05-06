#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <fui/fbi.h>

typedef struct player {
  float x, y;
  float orientation;
  float vx, vy;
} player_t;

typedef struct bullet {
  float x, y;
  float vx, vy;
} bullet_t;

typedef struct bullet_list {
  bullet_t *bullets;
  int count;
  int max_count;
  uint32_t cooldown;
  uint32_t last_shot;
  float speed;
} bullet_list_t;

extern bullet_list_t *bullet_list;
extern int score;

void init_player(player_t *player, float x, float y);
void draw_player(layer_t *layer, player_t *player);
void update_player(player_t *player, float dt, float min_x, float max_x,
                   float min_y, float max_y);
void shoot(player_t *player);
void update_bullets(bullet_list_t *bullet_list, float dt, float min_x,
                    float max_x, float min_y, float max_y);
void draw_bullets(layer_t *layer, bullet_list_t *bullet_list);
void destroy_bullet(bullet_list_t *bullet_list, int index);

#endif