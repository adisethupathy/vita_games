#include "entities.h"
#include "render.h"

void entities_spawn_for_room(Enemy enemies[MAX_ENEMIES], Interactables *it, const World *w, bool morph_already_owned) {
    memset(enemies, 0, sizeof(Enemy) * MAX_ENEMIES);
    memset(it, 0, sizeof(*it));
    for (int i = 0; i < w->enemy_count; ++i) {
        enemies[i].alive = true;
        enemies[i].box.x = w->enemy_spawns[i].x;
        enemies[i].box.y = w->enemy_spawns[i].y;
        enemies[i].box.w = 24.0f;
        enemies[i].box.h = 18.0f;
        enemies[i].vx = (i % 2 == 0) ? 60.0f : -60.0f;
    }
    if (w->has_morph_pickup && !morph_already_owned) {
        it->morph_present = true;
        it->morph_box = (Rect){w->morph_pickup.x, w->morph_pickup.y, 24.0f, 24.0f};
    }
}

void entities_update(Enemy enemies[MAX_ENEMIES], Interactables *it, Player *p, Shot shots[MAX_SHOTS], const World *w, float dt) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].alive) {
            continue;
        }
        enemies[i].anim_t += dt;
        enemies[i].box.x += enemies[i].vx * dt;

        /* turn at edges / walls */
        float feet_x = enemies[i].box.x + (enemies[i].vx > 0 ? enemies[i].box.w + 2.0f : -2.0f);
        float feet_y = enemies[i].box.y + enemies[i].box.h + 2.0f;
        int tx = (int)(feet_x / TILE);
        int ty = (int)(feet_y / TILE);
        int hx = (int)((enemies[i].box.x + enemies[i].box.w * 0.5f) / TILE);
        int hy = (int)((enemies[i].box.y + enemies[i].box.h * 0.5f) / TILE);
        if (!world_solid_for_player(w, tx, ty, false) || world_solid_for_player(w, hx + (enemies[i].vx > 0 ? 1 : -1), hy, false)) {
            enemies[i].vx = -enemies[i].vx;
        }

        if (p->hurt_timer <= 0.0f && rect_overlap(p->box, enemies[i].box)) {
            player_hurt(p, 1);
        }

        for (int s = 0; s < MAX_SHOTS; ++s) {
            if (!shots[s].alive) {
                continue;
            }
            if (rect_overlap(shots[s].box, enemies[i].box)) {
                enemies[i].alive = false;
                shots[s].alive = false;
                break;
            }
        }
    }

    if (it->morph_present && rect_overlap(p->box, it->morph_box)) {
        it->morph_present = false;
        p->has_morph = true;
    }

    if (w->has_exit) {
        Rect exit_r = {w->exit_pos.x, w->exit_pos.y, (float)TILE, (float)TILE};
        if (rect_overlap(p->box, exit_r)) {
            it->exit_touched = true;
        }
    }
}

void entities_draw(const Enemy enemies[MAX_ENEMIES], const Interactables *it, bool morph_owned, SDL_Renderer *r, const Textures *tex) {
    (void)morph_owned;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].alive) {
            continue;
        }
        int frame = ((int)(enemies[i].anim_t * 6.0f)) % 2;
        SDL_Rect src = {frame * 32, 0, 32, 24};
        bool flip = enemies[i].vx < 0;
        draw_texture(r, tex->enemy, &src, enemies[i].box.x - 4, enemies[i].box.y - 4, 32, 24, flip);
    }
    if (it->morph_present) {
        float bob = sinf(SDL_GetTicks() / 200.0f) * 3.0f;
        draw_texture(r, tex->pickup, NULL, it->morph_box.x, it->morph_box.y + bob, 24, 24, false);
    }
}
