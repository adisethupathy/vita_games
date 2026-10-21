#ifndef VARIA_ENTITIES_H
#define VARIA_ENTITIES_H

#include "common.h"
#include "player.h"
#include "render.h"
#include "world.h"

typedef struct {
    bool alive;
    Rect box;
    float vx;
    float anim_t;
} Enemy;

typedef struct {
    bool morph_present;
    Rect morph_box;
    bool exit_touched;
} Interactables;

void entities_spawn_for_room(Enemy enemies[MAX_ENEMIES], Interactables *it, const World *w, bool morph_already_owned);
void entities_update(Enemy enemies[MAX_ENEMIES], Interactables *it, Player *p, Shot shots[MAX_SHOTS], const World *w, float dt);
void entities_draw(const Enemy enemies[MAX_ENEMIES], const Interactables *it, bool morph_owned, SDL_Renderer *r, const Textures *tex);

#endif
