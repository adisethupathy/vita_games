#ifndef VARIA_PLAYER_H
#define VARIA_PLAYER_H

#include "common.h"
#include "world.h"

typedef struct {
    Rect box;
    Vec2 vel;
    float facing; /* -1 or 1 */
    bool on_ground;
    bool morph;
    bool has_morph;
    bool jumping;
    float coyote;
    float jump_buffer;
    float shoot_cd;
    int energy;
    float hurt_timer;
    float anim_t;
    int anim_frame;
} Player;

typedef struct {
    bool alive;
    Rect box;
    float vx;
    float life;
} Shot;

void player_spawn(Player *p, Vec2 pos);
void player_set_size(Player *p);
void player_update(Player *p, const World *w, float dt, float move, bool jump_pressed, bool jump_held, bool morph_held, bool shoot_pressed, Shot shots[MAX_SHOTS]);
void player_hurt(Player *p, int amount);
void player_draw(const Player *p, SDL_Renderer *r, SDL_Texture *tex);
void shots_update(Shot shots[MAX_SHOTS], const World *w, float dt);
void shots_draw(const Shot shots[MAX_SHOTS], SDL_Renderer *r, SDL_Texture *tex);

#endif
