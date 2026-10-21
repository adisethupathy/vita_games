/*
 * tiny-loop — minimal SDL2 game loop for PS Vita.
 *
 * The whole "framework" is just these steps:
 *   init → while running: input → update(dt) → render → quit
 *
 * Controls (Vita):
 *   Left stick / D-pad  — move the square
 *   Cross / Circle / Start — quit
 */

#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __vita__
#include <psp2/kernel/processmgr.h>
#endif

#define SCREEN_W 960
#define SCREEN_H 544

#define PLAYER_W 48
#define PLAYER_H 48
#define PLAYER_SPEED 320.0f /* pixels per second */
#define STICK_DEADZONE 8000

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_GameController *pad;
    bool running;
    Vec2 player;
    Vec2 input; /* -1..1 */
} Game;

static void game_init(Game *g) {
    g->window = NULL;
    g->renderer = NULL;
    g->pad = NULL;
    g->running = false;
    g->player.x = (SCREEN_W - PLAYER_W) * 0.5f;
    g->player.y = (SCREEN_H - PLAYER_H) * 0.5f;
    g->input.x = 0.0f;
    g->input.y = 0.0f;
}

static bool game_boot(Game *g) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    /* Vita has two touchscreens; don't invent mouse events from them. */
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");

    g->window = SDL_CreateWindow(
        "tiny-loop",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_W,
        SCREEN_H,
        0);
    if (!g->window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    g->renderer = SDL_CreateRenderer(
        g->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer) {
        g->renderer = SDL_CreateRenderer(g->window, -1, 0);
    }
    if (!g->renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
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

    g->running = true;
    return true;
}

static void game_shutdown(Game *g) {
    if (g->pad) {
        SDL_GameControllerClose(g->pad);
        g->pad = NULL;
    }
    if (g->renderer) {
        SDL_DestroyRenderer(g->renderer);
        g->renderer = NULL;
    }
    if (g->window) {
        SDL_DestroyWindow(g->window);
        g->window = NULL;
    }
    SDL_Quit();
}

static void game_handle_input(Game *g) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            g->running = false;
            break;
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                g->running = false;
            }
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            /* Xbox-style mapping: A=Cross, B=Circle on Vita SDL builds */
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A ||
                e.cbutton.button == SDL_CONTROLLER_BUTTON_B ||
                e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                g->running = false;
            }
            break;
        default:
            break;
        }
    }

    g->input.x = 0.0f;
    g->input.y = 0.0f;

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        g->input.x -= 1.0f;
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        g->input.x += 1.0f;
    }
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
        g->input.y -= 1.0f;
    }
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
        g->input.y += 1.0f;
    }

    if (g->pad) {
        const Sint16 ax = SDL_GameControllerGetAxis(g->pad, SDL_CONTROLLER_AXIS_LEFTX);
        const Sint16 ay = SDL_GameControllerGetAxis(g->pad, SDL_CONTROLLER_AXIS_LEFTY);
        float fx = 0.0f;
        float fy = 0.0f;
        if (ax > STICK_DEADZONE || ax < -STICK_DEADZONE) {
            fx = (float)ax / 32767.0f;
        }
        if (ay > STICK_DEADZONE || ay < -STICK_DEADZONE) {
            fy = (float)ay / 32767.0f;
        }

        if (SDL_GameControllerGetButton(g->pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
            fx = -1.0f;
        }
        if (SDL_GameControllerGetButton(g->pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
            fx = 1.0f;
        }
        if (SDL_GameControllerGetButton(g->pad, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
            fy = -1.0f;
        }
        if (SDL_GameControllerGetButton(g->pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
            fy = 1.0f;
        }

        if (fx != 0.0f || fy != 0.0f) {
            g->input.x = fx;
            g->input.y = fy;
        }
    }
}

static void game_update(Game *g, float dt) {
    g->player.x += g->input.x * PLAYER_SPEED * dt;
    g->player.y += g->input.y * PLAYER_SPEED * dt;

    if (g->player.x < 0.0f) {
        g->player.x = 0.0f;
    }
    if (g->player.y < 0.0f) {
        g->player.y = 0.0f;
    }
    if (g->player.x > (float)(SCREEN_W - PLAYER_W)) {
        g->player.x = (float)(SCREEN_W - PLAYER_W);
    }
    if (g->player.y > (float)(SCREEN_H - PLAYER_H)) {
        g->player.y = (float)(SCREEN_H - PLAYER_H);
    }
}

static void game_render(Game *g) {
    SDL_SetRenderDrawColor(g->renderer, 12, 28, 36, 255);
    SDL_RenderClear(g->renderer);

    SDL_Rect dst = {
        (int)g->player.x,
        (int)g->player.y,
        PLAYER_W,
        PLAYER_H};
    SDL_SetRenderDrawColor(g->renderer, 255, 196, 72, 255);
    SDL_RenderFillRect(g->renderer, &dst);

    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(g->renderer, &dst);

    SDL_RenderPresent(g->renderer);
}

static void game_run(Game *g) {
    Uint64 prev = SDL_GetPerformanceCounter();
    const double freq = (double)SDL_GetPerformanceFrequency();

    while (g->running) {
        const Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((double)(now - prev) / freq);
        prev = now;
        if (dt > 0.1f) {
            dt = 0.1f;
        }

        game_handle_input(g);
        game_update(g, dt);
        game_render(g);
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    Game game;
    game_init(&game);

    if (!game_boot(&game)) {
        game_shutdown(&game);
#ifdef __vita__
        sceKernelExitProcess(1);
#endif
        return 1;
    }

    game_run(&game);
    game_shutdown(&game);

#ifdef __vita__
    sceKernelExitProcess(0);
#endif
    return 0;
}
