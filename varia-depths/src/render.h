#ifndef VARIA_RENDER_H
#define VARIA_RENDER_H

#include "common.h"

typedef struct {
    SDL_Texture *player;
    SDL_Texture *enemy;
    SDL_Texture *tiles;
    SDL_Texture *beam;
    SDL_Texture *pickup;
    SDL_Texture *hud;
    SDL_Texture *bg_grad;
    SDL_Texture *bg_sil;
} Textures;

bool textures_load(Textures *t, SDL_Renderer *r);
void textures_destroy(Textures *t);

void draw_texture(SDL_Renderer *r, SDL_Texture *tex, SDL_Rect *src, float x, float y, float w, float h, bool flip);
void draw_bg(SDL_Renderer *r, const Textures *t, float parallax_x);

#endif
