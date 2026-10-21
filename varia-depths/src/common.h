#ifndef VARIA_COMMON_H
#define VARIA_COMMON_H

#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __vita__
#include <psp2/kernel/processmgr.h>
#define ASSET_PREFIX "app0:assets/"
#else
#define ASSET_PREFIX "assets/"
#endif

#define SCREEN_W 960
#define SCREEN_H 544
#define TILE 32
#define ROOM_W 30
#define ROOM_H 17

#define MAX_ENEMIES 16
#define MAX_SHOTS 8
#define MAX_ROOMS 6

#define PLAYER_STAND_W 20.0f
#define PLAYER_STAND_H 28.0f
#define PLAYER_MORPH_W 16.0f
#define PLAYER_MORPH_H 16.0f

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    float x, y, w, h;
} Rect;

static inline float clampf(float v, float lo, float hi) {
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

static inline bool rect_overlap(Rect a, Rect b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

#endif
