#ifndef VARIA_GAME_H
#define VARIA_GAME_H

#include "common.h"
#include "entities.h"
#include "player.h"
#include "render.h"
#include "world.h"

typedef enum {
    MODE_TITLE,
    MODE_PLAY,
    MODE_DEAD,
    MODE_WIN
} GameMode;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_GameController *pad;
    Textures tex;
    World world;
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Interactables interact;
    Shot shots[MAX_SHOTS];
    GameMode mode;
    bool running;
    bool morph_owned_global;
} Game;

bool game_boot(Game *g);
void game_shutdown(Game *g);
void game_run(Game *g);

#endif
