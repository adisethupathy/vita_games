#include "render.h"

static SDL_Texture *load_tex(SDL_Renderer *r, const char *name) {
    char path[256];
    snprintf(path, sizeof(path), "%s%s", ASSET_PREFIX, name);
    SDL_Texture *tex = IMG_LoadTexture(r, path);
    if (!tex) {
        /* Desktop fallback when run from build dir */
        snprintf(path, sizeof(path), "../assets/%s", name);
        tex = IMG_LoadTexture(r, path);
    }
    if (!tex) {
        snprintf(path, sizeof(path), "assets/%s", name);
        tex = IMG_LoadTexture(r, path);
    }
    if (!tex) {
        fprintf(stderr, "Failed to load %s: %s\n", name, IMG_GetError());
    }
    return tex;
}

bool textures_load(Textures *t, SDL_Renderer *r) {
    memset(t, 0, sizeof(*t));
    t->player = load_tex(r, "player.png");
    t->enemy = load_tex(r, "enemy_crawler.png");
    t->tiles = load_tex(r, "tiles.png");
    t->beam = load_tex(r, "beam.png");
    t->pickup = load_tex(r, "pickup_morph.png");
    t->hud = load_tex(r, "hud_energy.png");
    t->bg_grad = load_tex(r, "bg_gradient.png");
    t->bg_sil = load_tex(r, "bg_silhouettes.png");
    return t->player && t->enemy && t->tiles && t->beam && t->pickup && t->hud && t->bg_grad && t->bg_sil;
}

void textures_destroy(Textures *t) {
    SDL_Texture **list[] = {
        &t->player, &t->enemy, &t->tiles, &t->beam, &t->pickup, &t->hud, &t->bg_grad, &t->bg_sil};
    for (size_t i = 0; i < sizeof(list) / sizeof(list[0]); ++i) {
        if (*list[i]) {
            SDL_DestroyTexture(*list[i]);
            *list[i] = NULL;
        }
    }
}

void draw_texture(SDL_Renderer *r, SDL_Texture *tex, SDL_Rect *src, float x, float y, float w, float h, bool flip) {
    SDL_Rect dst = {(int)x, (int)y, (int)w, (int)h};
    SDL_RendererFlip f = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(r, tex, src, &dst, 0.0, NULL, f);
}

void draw_bg(SDL_Renderer *r, const Textures *t, float parallax_x) {
    SDL_Rect full = {0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderCopy(r, t->bg_grad, NULL, &full);

    int offset = (int)(parallax_x * 0.2f) % SCREEN_W;
    if (offset < 0) {
        offset += SCREEN_W;
    }
    SDL_Rect a = {-offset, 0, SCREEN_W, SCREEN_H};
    SDL_Rect b = {-offset + SCREEN_W, 0, SCREEN_W, SCREEN_H};
    SDL_RenderCopy(r, t->bg_sil, NULL, &a);
    SDL_RenderCopy(r, t->bg_sil, NULL, &b);
}
