#include "player.h"
#include "render.h"

#define MOVE_ACCEL 2200.0f
#define MOVE_MAX 220.0f
#define GROUND_FRICTION 1800.0f
#define GRAVITY 900.0f
#define JUMP_V -380.0f
#define COYOTE_T 0.1f
#define JUMP_BUF 0.1f
#define SHOOT_CD 0.22f
#define MAX_ENERGY 5

void player_set_size(Player *p) {
    float cx = p->box.x + p->box.w * 0.5f;
    float bottom = p->box.y + p->box.h;
    if (p->morph) {
        p->box.w = PLAYER_MORPH_W;
        p->box.h = PLAYER_MORPH_H;
    } else {
        p->box.w = PLAYER_STAND_W;
        p->box.h = PLAYER_STAND_H;
    }
    p->box.x = cx - p->box.w * 0.5f;
    p->box.y = bottom - p->box.h;
}

void player_spawn(Player *p, Vec2 pos) {
    memset(p, 0, sizeof(*p));
    p->box.x = pos.x;
    p->box.y = pos.y;
    p->box.w = PLAYER_STAND_W;
    p->box.h = PLAYER_STAND_H;
    p->facing = 1.0f;
    p->energy = MAX_ENERGY;
    p->has_morph = false;
    p->morph = false;
}

void player_hurt(Player *p, int amount) {
    if (p->hurt_timer > 0.0f) {
        return;
    }
    p->energy -= amount;
    if (p->energy < 0) {
        p->energy = 0;
    }
    p->hurt_timer = 1.0f;
    p->vel.y = -180.0f;
    p->vel.x = -p->facing * 120.0f;
}

static void spawn_shot(Shot shots[MAX_SHOTS], const Player *p) {
    for (int i = 0; i < MAX_SHOTS; ++i) {
        if (shots[i].alive) {
            continue;
        }
        shots[i].alive = true;
        shots[i].life = 0.7f;
        shots[i].vx = p->facing * 420.0f;
        float y = p->morph ? p->box.y + 4.0f : p->box.y + 10.0f;
        float x = p->facing > 0 ? p->box.x + p->box.w : p->box.x - 16.0f;
        shots[i].box = (Rect){x, y, 16.0f, 6.0f};
        return;
    }
}

void player_update(Player *p, const World *w, float dt, float move, bool jump_pressed, bool jump_held, bool morph_held, bool shoot_pressed, Shot shots[MAX_SHOTS]) {
    if (p->hurt_timer > 0.0f) {
        p->hurt_timer -= dt;
    }
    if (p->shoot_cd > 0.0f) {
        p->shoot_cd -= dt;
    }

    bool want_morph = p->has_morph && morph_held;
    if (want_morph != p->morph) {
        bool old = p->morph;
        p->morph = want_morph;
        player_set_size(p);
        /* If unmorph blocked, stay morph */
        if (!p->morph) {
            Rect test = p->box;
            Vec2 zero = {0, 0};
            bool ground = false;
            Vec2 v = zero;
            /* probe: if current rect overlaps solid when standing, revert */
            int x0 = (int)floorf(test.x / TILE);
            int y0 = (int)floorf(test.y / TILE);
            int x1 = (int)floorf((test.x + test.w - 1) / TILE);
            int y1 = (int)floorf((test.y + test.h - 1) / TILE);
            bool blocked = false;
            for (int ty = y0; ty <= y1 && !blocked; ++ty) {
                for (int tx = x0; tx <= x1; ++tx) {
                    if (world_solid_for_player(w, tx, ty, false)) {
                        blocked = true;
                    }
                }
            }
            (void)v;
            (void)ground;
            if (blocked) {
                p->morph = old;
                player_set_size(p);
            }
        }
    }

    float max_spd = p->morph ? MOVE_MAX * 0.7f : MOVE_MAX;
    if (move != 0.0f) {
        p->facing = move > 0 ? 1.0f : -1.0f;
        p->vel.x += move * MOVE_ACCEL * dt;
        p->vel.x = clampf(p->vel.x, -max_spd, max_spd);
    } else if (p->on_ground) {
        if (p->vel.x > 0) {
            p->vel.x = fmaxf(0.0f, p->vel.x - GROUND_FRICTION * dt);
        } else if (p->vel.x < 0) {
            p->vel.x = fminf(0.0f, p->vel.x + GROUND_FRICTION * dt);
        }
    }

    if (jump_pressed) {
        p->jump_buffer = JUMP_BUF;
    } else {
        p->jump_buffer -= dt;
    }
    if (p->on_ground) {
        p->coyote = COYOTE_T;
    } else {
        p->coyote -= dt;
    }

    if (!p->morph && p->jump_buffer > 0.0f && p->coyote > 0.0f) {
        p->vel.y = JUMP_V;
        p->on_ground = false;
        p->coyote = 0.0f;
        p->jump_buffer = 0.0f;
        p->jumping = true;
    }
    if (p->jumping && !jump_held && p->vel.y < 0.0f) {
        p->vel.y *= 0.45f;
        p->jumping = false;
    }
    if (p->vel.y >= 0.0f) {
        p->jumping = false;
    }

    p->vel.y += GRAVITY * dt;
    if (p->vel.y > 600.0f) {
        p->vel.y = 600.0f;
    }

    world_move_and_collide(w, &p->box, &p->vel, p->morph, &p->on_ground);

    if (!p->morph && shoot_pressed && p->shoot_cd <= 0.0f) {
        spawn_shot(shots, p);
        p->shoot_cd = SHOOT_CD;
    }

    /* anim */
    p->anim_t += dt;
    if (p->morph) {
        p->anim_frame = 4;
    } else if (!p->on_ground) {
        p->anim_frame = 3;
    } else if (fabsf(p->vel.x) > 20.0f) {
        p->anim_frame = (int)(p->anim_t * 10.0f) % 2 + 1;
    } else {
        p->anim_frame = 0;
    }

    /* spikes */
    int cx = (int)((p->box.x + p->box.w * 0.5f) / TILE);
    int cy = (int)((p->box.y + p->box.h - 2.0f) / TILE);
    if (world_tile_at(w, cx, cy) == TILE_SPIKE) {
        player_hurt(p, 1);
    }
}

void player_draw(const Player *p, SDL_Renderer *r, SDL_Texture *tex) {
    if (p->hurt_timer > 0.0f && fmodf(p->hurt_timer, 0.1f) < 0.05f) {
        return; /* flicker */
    }
    SDL_Rect src = {p->anim_frame * 32, 0, 32, 32};
    float dw = p->morph ? 24.0f : 32.0f;
    float dh = p->morph ? 24.0f : 32.0f;
    float dx = p->box.x + p->box.w * 0.5f - dw * 0.5f;
    float dy = p->box.y + p->box.h - dh;
    draw_texture(r, tex, &src, dx, dy, dw, dh, p->facing < 0);
}

void shots_update(Shot shots[MAX_SHOTS], const World *w, float dt) {
    for (int i = 0; i < MAX_SHOTS; ++i) {
        if (!shots[i].alive) {
            continue;
        }
        shots[i].life -= dt;
        shots[i].box.x += shots[i].vx * dt;
        if (shots[i].life <= 0.0f) {
            shots[i].alive = false;
            continue;
        }
        int tx = (int)(shots[i].box.x / TILE);
        int ty = (int)(shots[i].box.y / TILE);
        TileType t = world_tile_at(w, tx, ty);
        if (t == TILE_SOLID || t == TILE_DECO || t == TILE_TUNNEL) {
            shots[i].alive = false;
        }
    }
}

void shots_draw(const Shot shots[MAX_SHOTS], SDL_Renderer *r, SDL_Texture *tex) {
    for (int i = 0; i < MAX_SHOTS; ++i) {
        if (!shots[i].alive) {
            continue;
        }
        bool flip = shots[i].vx < 0;
        draw_texture(r, tex, NULL, shots[i].box.x, shots[i].box.y, shots[i].box.w, shots[i].box.h, flip);
    }
}
