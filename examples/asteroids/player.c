#include "player.h"
#include <fui/clock.h>
#include <fui/debugui.h>
#include <fui/fbi.h>
#include <fui/primitives.h>
#include <math.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846
// vtx 0 is the tip, forward direction of the player
#define vtx_0 0.0f
#define vty_0 15.0f
#define vtx_1 -7.5f
#define vty_1 -7.5f
#define vtx_2 7.5f
#define vty_2 -7.5f

int score = 0;
bullet_list_t *bullet_list;

void init_player(player_t *player, float x, float y) {
  player->x = x;
  player->y = y;
  player->orientation = 0.0f;
  player->vx = 0.0f;
  player->vy = 0.0f;
  bullet_list = malloc(sizeof(bullet_list_t));
  bullet_list->bullets = malloc(sizeof(bullet_t) * 100);
  bullet_list->count = 0;
  bullet_list->max_count = 100;
  bullet_list->cooldown = 100; // ms
  bullet_list->last_shot = 0;
  bullet_list->speed = 1000.0f; // pixels per second
}

void draw_player(layer_t *layer, player_t *player) {
  float angle = player->orientation;
  float rotated_x1 = cosf(angle) * vtx_0 - sinf(angle) * vty_0;
  float rotated_y1 = sinf(angle) * vtx_0 + cosf(angle) * vty_0;
  float rotated_x2 = cosf(angle) * vtx_1 - sinf(angle) * vty_1;
  float rotated_y2 = sinf(angle) * vtx_1 + cosf(angle) * vty_1;
  float rotated_x3 = cosf(angle) * vtx_2 - sinf(angle) * vty_2;
  float rotated_y3 = sinf(angle) * vtx_2 + cosf(angle) * vty_2;
  draw_line(layer, player->x + rotated_x1, player->y + rotated_y1,
            player->x + rotated_x2, player->y + rotated_y2, 0xFFFF0000);
  draw_line(layer, player->x + rotated_x2, player->y + rotated_y2,
            player->x + rotated_x3, player->y + rotated_y3, 0xFFFF0000);
  draw_line(layer, player->x + rotated_x3, player->y + rotated_y3,
            player->x + rotated_x1, player->y + rotated_y1, 0xFFFF0000);
}

void update_player(player_t *player, float dt, float min_x, float max_x,
                   float min_y, float max_y) {
  player->x += player->vx * dt;
  player->y += player->vy * dt;
  if (player->x < min_x) {
    player->x = min_x;
    player->vx = 0.0f;
  } else if (player->x > max_x) {
    player->x = max_x;
    player->vx = 0.0f;
  }
  if (player->y < min_y) {
    player->y = min_y;
    player->vy = 0.0f;
  } else if (player->y > max_y) {
    player->y = max_y;
    player->vy = 0.0f;
  }
  // damp speed
  player->vx *= 0.995f;
  player->vy *= 0.995f;
}

void shoot(player_t *player) {
  if (bullet_list->count >= 100) {
    return;
  }
  if (get_time_ms() - bullet_list->last_shot < bullet_list->cooldown) {
    return;
  }
  bullet_list->bullets[bullet_list->count].x = player->x;
  bullet_list->bullets[bullet_list->count].y = player->y;
  bullet_list->bullets[bullet_list->count].vx =
      -sinf(player->orientation) * bullet_list->speed;
  bullet_list->bullets[bullet_list->count].vy =
      cosf(player->orientation) * bullet_list->speed;
  bullet_list->last_shot = get_time_ms();
  bullet_list->count++;
}

void update_bullets(bullet_list_t *bullet_list, float dt, float min_x,
                    float max_x, float min_y, float max_y) {
  for (int i = 0; i < bullet_list->count; i++) {
    bullet_t *b = &bullet_list->bullets[i];
    b->x += b->vx * dt;
    b->y += b->vy * dt;
    if (b->x < min_x || b->x > max_x || b->y < min_y || b->y > max_y) {
      destroy_bullet(bullet_list, i);
      if (i == bullet_list->count || bullet_list->count == 0) {
        break;
      }
      i--;
    }
  }
}

void draw_bullets(layer_t *layer, bullet_list_t *bullet_list) {
  for (int i = 0; i < bullet_list->count; i++) {
    bullet_t *b = &bullet_list->bullets[i];
    draw_rectangle(layer, b->x - 2, b->y - 2, 4, 4, 0xFF00FF00, true);
  }
}

void destroy_bullet(bullet_list_t *bullet_list, int index) {
  if (index < 0 || index >= bullet_list->count) {
    return;
  }
  for (int i = index; i < bullet_list->count - 1; i++) {
    bullet_list->bullets[i] = bullet_list->bullets[i + 1];
  }
  bullet_list->count--;
}