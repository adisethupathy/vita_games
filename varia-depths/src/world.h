#ifndef VARIA_WORLD_H
#define VARIA_WORLD_H

#include "common.h"

typedef enum {
    TILE_EMPTY = 0,
    TILE_SOLID,
    TILE_DOOR,
    TILE_SPIKE,
    TILE_TUNNEL, /* morph-only passage solids above/below crawl space */
    TILE_EXIT,
    TILE_DECO
} TileType;

typedef struct {
    int id;
    const char *name;
    const char *rows[ROOM_H];
    int door_left;  /* room id or -1 */
    int door_right; /* room id or -1 */
} RoomDef;

typedef struct {
    int room_id;
    TileType tiles[ROOM_H][ROOM_W];
    Vec2 spawn;
    Vec2 morph_pickup;
    bool has_morph_pickup;
    Vec2 exit_pos;
    bool has_exit;
    int enemy_count;
    Vec2 enemy_spawns[MAX_ENEMIES];
} World;

void world_init(World *w);
void world_load_room(World *w, int room_id);
TileType world_tile_at(const World *w, int tx, int ty);
bool world_solid_for_player(const World *w, int tx, int ty, bool morph);
bool world_move_and_collide(const World *w, Rect *box, Vec2 *vel, bool morph, bool *on_ground);
int world_try_door(const World *w, Rect box, float facing);
void world_draw(const World *w, SDL_Renderer *r, SDL_Texture *tiles_tex);

#endif
