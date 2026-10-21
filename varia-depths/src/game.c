#include "game.h"

static void enter_room(Game *g, int room_id, float from_facing) {
    world_load_room(&g->world, room_id);
    Vec2 spawn = g->world.spawn;
    if (from_facing > 0) {
        /* entered from left door → spawn near left */
        spawn.x = TILE * 2.0f;
    } else if (from_facing < 0) {
        spawn.x = (ROOM_W - 3) * TILE;
    }
    spawn.y = g->player.box.y;
    if (spawn.y < TILE) {
        spawn.y = g->world.spawn.y;
    }
    bool keep_morph = g->player.has_morph || g->morph_owned_global;
    int energy = g->player.energy > 0 ? g->player.energy : 5;
    player_spawn(&g->player, spawn);
    g->player.has_morph = keep_morph;
    g->player.energy = energy;
    g->morph_owned_global = keep_morph;
    memset(g->shots, 0, sizeof(g->shots));
    entities_spawn_for_room(g->enemies, &g->interact, &g->world, g->morph_owned_global);
}

static void reset_game(Game *g) {
    g->morph_owned_global = false;
    g->mode = MODE_PLAY;
    world_init(&g->world);
    player_spawn(&g->player, g->world.spawn);
    memset(g->shots, 0, sizeof(g->shots));
    entities_spawn_for_room(g->enemies, &g->interact, &g->world, false);
}

bool game_boot(Game *g) {
    memset(g, 0, sizeof(*g));
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    int img_flags = IMG_INIT_PNG;
    if ((IMG_Init(img_flags) & img_flags) == 0) {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        return false;
    }
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    g->window = SDL_CreateWindow("Varia Depths", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, 0);
    if (!g->window) {
        return false;
    }
    g->renderer = SDL_CreateRenderer(g->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer) {
        g->renderer = SDL_CreateRenderer(g->window, -1, 0);
    }
    if (!g->renderer) {
        return false;
    }
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            g->pad = SDL_GameControllerOpen(i);
            if (g->pad) {
                break;
            }
        }
    }
    if (!textures_load(&g->tex, g->renderer)) {
        return false;
    }
    g->mode = MODE_TITLE;
    g->running = true;
    return true;
}

void game_shutdown(Game *g) {
    textures_destroy(&g->tex);
    if (g->pad) {
        SDL_GameControllerClose(g->pad);
    }
    if (g->renderer) {
        SDL_DestroyRenderer(g->renderer);
    }
    if (g->window) {
        SDL_DestroyWindow(g->window);
    }
    IMG_Quit();
    SDL_Quit();
}

static void read_input(Game *g, float *move, bool *jump_p, bool *jump_h, bool *morph_h, bool *shoot_p, bool *start_p, bool *confirm_p) {
    *move = 0;
    *jump_p = false;
    *jump_h = false;
    *morph_h = false;
    *shoot_p = false;
    *start_p = false;
    *confirm_p = false;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g->running = false;
        } else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                *start_p = true;
            }
            if (e.key.keysym.sym == SDLK_z || e.key.keysym.sym == SDLK_SPACE) {
                *jump_p = true;
            }
            if (e.key.keysym.sym == SDLK_x) {
                *shoot_p = true;
            }
            if (e.key.keysym.sym == SDLK_RETURN) {
                *confirm_p = true;
            }
        } else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
                *jump_p = true;
            }
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_X || e.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
                *shoot_p = true;
            }
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                *start_p = true;
            }
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A || e.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
                *confirm_p = true;
            }
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        *move -= 1.0f;
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        *move += 1.0f;
    }
    *jump_h = keys[SDL_SCANCODE_Z] || keys[SDL_SCANCODE_SPACE];
    *morph_h = keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S];

    if (g->pad) {
        Sint16 ax = SDL_GameControllerGetAxis(g->pad, SDL_CONTROLLER_AXIS_LEFTX);
        if (ax > 8000) {
            *move = 1.0f;
        } else if (ax < -8000) {
            *move = -1.0f;
        }
        if (SDL_GameControllerGetButton(g->pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
            *move = -1.0f;
        }
        if (SDL_GameControllerGetButton(g->pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
            *move = 1.0f;
        }
        if (SDL_GameControllerGetButton(g->pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
            *morph_h = true;
        }
        if (SDL_GameControllerGetButton(g->pad, SDL_CONTROLLER_BUTTON_A)) {
            *jump_h = true;
        }
    }
}

static void draw_hud(Game *g) {
    for (int i = 0; i < g->player.energy; ++i) {
        draw_texture(g->renderer, g->tex.hud, NULL, 16.0f + i * 16.0f, 12.0f, 12.0f, 12.0f, false);
    }
    if (g->player.has_morph) {
        draw_texture(g->renderer, g->tex.pickup, NULL, SCREEN_W - 40.0f, 10.0f, 20.0f, 20.0f, false);
    }
}

static void draw_center_text_proxy(Game *g, const char *line1, const char *line2) {
    /* No TTF — use colored bars + block hint via filled rects as "title card" */
    (void)line1;
    (void)line2;
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 180);
    SDL_Rect panel = {SCREEN_W / 2 - 220, SCREEN_H / 2 - 80, 440, 160};
    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(g->renderer, &panel);
    SDL_SetRenderDrawColor(g->renderer, 220, 170, 70, 255);
    SDL_Rect title = {SCREEN_W / 2 - 160, SCREEN_H / 2 - 50, 320, 24};
    SDL_RenderFillRect(g->renderer, &title);
    SDL_SetRenderDrawColor(g->renderer, 80, 220, 255, 255);
    SDL_Rect sub = {SCREEN_W / 2 - 120, SCREEN_H / 2 + 10, 240, 12};
    SDL_RenderFillRect(g->renderer, &sub);
}

void game_run(Game *g) {
    Uint64 prev = SDL_GetPerformanceCounter();
    const double freq = (double)SDL_GetPerformanceFrequency();

    while (g->running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((double)(now - prev) / freq);
        prev = now;
        if (dt > 0.05f) {
            dt = 0.05f;
        }

        float move;
        bool jump_p, jump_h, morph_h, shoot_p, start_p, confirm_p;
        read_input(g, &move, &jump_p, &jump_h, &morph_h, &shoot_p, &start_p, &confirm_p);

        if (g->mode == MODE_TITLE) {
            if (confirm_p || jump_p) {
                reset_game(g);
            }
            if (start_p) {
                g->running = false;
            }
        } else if (g->mode == MODE_DEAD || g->mode == MODE_WIN) {
            if (confirm_p || jump_p) {
                g->mode = MODE_TITLE;
            }
            if (start_p) {
                g->running = false;
            }
        } else if (g->mode == MODE_PLAY) {
            if (start_p) {
                g->mode = MODE_TITLE;
            }
            player_update(&g->player, &g->world, dt, move, jump_p, jump_h, morph_h, shoot_p, g->shots);
            shots_update(g->shots, &g->world, dt);
            entities_update(g->enemies, &g->interact, &g->player, g->shots, &g->world, dt);
            if (g->player.has_morph) {
                g->morph_owned_global = true;
            }

            int next = world_try_door(&g->world, g->player.box, g->player.facing);
            if (next >= 0 && (jump_p || fabsf(g->player.vel.x) > 40.0f || move != 0.0f)) {
                /* require overlap near edge */
                bool near_left = g->player.box.x < TILE * 1.5f;
                bool near_right = g->player.box.x + g->player.box.w > (ROOM_W - 1.5f) * TILE;
                if ((near_left && g->player.facing < 0) || (near_right && g->player.facing > 0)) {
                    float face = g->player.facing;
                    enter_room(g, next, face);
                }
            }

            if (g->interact.exit_touched) {
                g->mode = MODE_WIN;
            }
            if (g->player.energy <= 0) {
                g->mode = MODE_DEAD;
            }
        }

        /* render */
        draw_bg(g->renderer, &g->tex, g->player.box.x);
        if (g->mode == MODE_PLAY || g->mode == MODE_DEAD || g->mode == MODE_WIN) {
            world_draw(&g->world, g->renderer, g->tex.tiles);
            entities_draw(g->enemies, &g->interact, g->morph_owned_global, g->renderer, &g->tex);
            player_draw(&g->player, g->renderer, g->tex.player);
            shots_draw(g->shots, g->renderer, g->tex.beam);
            draw_hud(g);
        }
        if (g->mode == MODE_TITLE) {
            draw_center_text_proxy(g, "VARIA DEPTHS", "Press Cross");
            /* gold title already drawn; add accent morph icon */
            draw_texture(g->renderer, g->tex.player, &(SDL_Rect){0, 0, 32, 32}, SCREEN_W / 2 - 32, SCREEN_H / 2 - 120, 64, 64, false);
        } else if (g->mode == MODE_DEAD) {
            draw_center_text_proxy(g, "ENERGY DEPLETED", "Retry");
        } else if (g->mode == MODE_WIN) {
            draw_center_text_proxy(g, "ESCAPE COMPLETE", "Title");
        }

        SDL_RenderPresent(g->renderer);
    }
}
