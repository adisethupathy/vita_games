#include "world.h"

/* 30×17 rooms. Legend:
 * . empty  # solid  D door  S spawn  E enemy  M morph  X exit  ^ spike  ~ tunnel block
 * = decorative wall
 */

static const RoomDef ROOMS[MAX_ROOMS] = {
    {
        .id = 0,
        .name = "Landing Shaft",
        .door_left = -1,
        .door_right = 1,
        .rows =
            {
                "##############################",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#..............####..........#",
                "#............................#",
                "#.........^^.................#",
                "#.......######...............#",
                "#............................#",
                "#S..........................D#",
                "##############################",
                "##############################",
                "##############################",
                "##############################",
            },
    },
    {
        .id = 1,
        .name = "Crawl Corridor",
        .door_left = 0,
        .door_right = 2,
        .rows =
            {
                "##############################",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#....E.......................#",
                "#..######............E.......#",
                "#.................######.....#",
                "#............................#",
                "#............................#",
                "#........^^..................#",
                "#......######................#",
                "D...........................D#",
                "##############################",
                "##############################",
                "##############################",
                "##############################",
            },
    },
    {
        .id = 2,
        .name = "Morph Cache",
        .door_left = 1,
        .door_right = 3,
        .rows =
            {
                "##############################",
                "#............................#",
                "#............................#",
                "#............M...............#",
                "#..........####..............#",
                "#............................#",
                "#............................#",
                "#....E.......................#",
                "#..######....................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "D...........................D#",
                "##############################",
                "##############################",
                "##############################",
                "##############################",
            },
    },
    {
        .id = 3,
        .name = "Tunnel Gate",
        .door_left = 2,
        .door_right = 4,
        .rows =
            {
                "##############################",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#..............~~............#",
                "#..........E...~~............#",
                "#........####..~~............#",
                "#..............~~............#",
                "#..............~~............#",
                "D..............~~...........D#",
                "##############################",
                "##############################",
                "##############################",
                "##############################",
            },
    },
    {
        .id = 4,
        .name = "Ascent",
        .door_left = 3,
        .door_right = 5,
        .rows =
            {
                "##############################",
                "#............................#",
                "#...................####.....#",
                "#............................#",
                "#.............####...........#",
                "#............................#",
                "#.......####.................#",
                "#............................#",
                "#...####.....................#",
                "#............................#",
                "#.................E..........#",
                "#...............####.........#",
                "D...........................D#",
                "##############################",
                "##############################",
                "##############################",
                "##############################",
            },
    },
    {
        .id = 5,
        .name = "Surface Exit",
        .door_left = 4,
        .door_right = -1,
        .rows =
            {
                "##############################",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#............................#",
                "#..............X.............#",
                "#.............####...........#",
                "#............................#",
                "#............................#",
                "#............................#",
                "D............................#",
                "##############################",
                "##############################",
                "##############################",
                "##############################",
            },
    },
};

static TileType char_to_tile(char c) {
    switch (c) {
    case '#':
        return TILE_SOLID;
    case 'D':
        return TILE_DOOR;
    case '^':
        return TILE_SPIKE;
    case '~':
        return TILE_TUNNEL;
    case 'X':
        return TILE_EXIT;
    case '=':
        return TILE_DECO;
    default:
        return TILE_EMPTY;
    }
}

void world_init(World *w) {
    memset(w, 0, sizeof(*w));
    world_load_room(w, 0);
}

void world_load_room(World *w, int room_id) {
    if (room_id < 0 || room_id >= MAX_ROOMS) {
        room_id = 0;
    }
    memset(w, 0, sizeof(*w));
    w->room_id = room_id;
    const RoomDef *def = &ROOMS[room_id];
    w->spawn.x = TILE * 2.0f;
    w->spawn.y = TILE * 10.0f;

    for (int y = 0; y < ROOM_H; ++y) {
        const char *row = def->rows[y];
        for (int x = 0; x < ROOM_W; ++x) {
            char c = row[x];
            w->tiles[y][x] = char_to_tile(c);
            if (c == 'S') {
                w->spawn.x = (float)(x * TILE);
                w->spawn.y = (float)(y * TILE);
            } else if (c == 'M') {
                w->has_morph_pickup = true;
                w->morph_pickup.x = (float)(x * TILE + 4);
                w->morph_pickup.y = (float)(y * TILE + 4);
            } else if (c == 'X') {
                w->has_exit = true;
                w->exit_pos.x = (float)(x * TILE);
                w->exit_pos.y = (float)(y * TILE);
            } else if (c == 'E' && w->enemy_count < MAX_ENEMIES) {
                w->enemy_spawns[w->enemy_count].x = (float)(x * TILE);
                w->enemy_spawns[w->enemy_count].y = (float)(y * TILE);
                w->enemy_count++;
            }
        }
    }
}

TileType world_tile_at(const World *w, int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= ROOM_W || ty >= ROOM_H) {
        return TILE_SOLID;
    }
    return w->tiles[ty][tx];
}

bool world_solid_for_player(const World *w, int tx, int ty, bool morph) {
    TileType t = world_tile_at(w, tx, ty);
    if (t == TILE_SOLID || t == TILE_DECO || t == TILE_EXIT) {
        return true;
    }
    if (t == TILE_TUNNEL) {
        /* Standing player collides; morph can pass */
        return !morph;
    }
    return false;
}

static bool rect_hits_solid(const World *w, Rect box, bool morph) {
    int x0 = (int)floorf(box.x / TILE);
    int y0 = (int)floorf(box.y / TILE);
    int x1 = (int)floorf((box.x + box.w - 1.0f) / TILE);
    int y1 = (int)floorf((box.y + box.h - 1.0f) / TILE);
    for (int ty = y0; ty <= y1; ++ty) {
        for (int tx = x0; tx <= x1; ++tx) {
            if (world_solid_for_player(w, tx, ty, morph)) {
                return true;
            }
        }
    }
    return false;
}

bool world_move_and_collide(const World *w, Rect *box, Vec2 *vel, bool morph, bool *on_ground) {
    *on_ground = false;

    box->x += vel->x;
    if (rect_hits_solid(w, *box, morph)) {
        if (vel->x > 0) {
            int tx = (int)floorf((box->x + box->w - 0.01f) / TILE);
            box->x = (float)tx * TILE - box->w;
        } else if (vel->x < 0) {
            int tx = (int)floorf(box->x / TILE);
            box->x = (float)(tx + 1) * TILE;
        }
        vel->x = 0;
    }

    box->y += vel->y;
    if (rect_hits_solid(w, *box, morph)) {
        if (vel->y > 0) {
            int ty = (int)floorf((box->y + box->h - 0.01f) / TILE);
            box->y = (float)ty * TILE - box->h;
            *on_ground = true;
        } else if (vel->y < 0) {
            int ty = (int)floorf(box->y / TILE);
            box->y = (float)(ty + 1) * TILE;
        }
        vel->y = 0;
    }

    /* Ground probe */
    Rect probe = *box;
    probe.y += 1.0f;
    if (rect_hits_solid(w, probe, morph)) {
        *on_ground = true;
    }
    return true;
}

int world_try_door(const World *w, Rect box, float facing) {
    const RoomDef *def = &ROOMS[w->room_id];
    int cx = (int)((box.x + box.w * 0.5f) / TILE);
    int cy = (int)((box.y + box.h * 0.5f) / TILE);
    if (world_tile_at(w, cx, cy) != TILE_DOOR && world_tile_at(w, cx, cy + 1) != TILE_DOOR) {
        /* also check edges */
        if (box.x < 8.0f && facing < 0 && def->door_left >= 0) {
            return def->door_left;
        }
        if (box.x + box.w > ROOM_W * TILE - 8.0f && facing > 0 && def->door_right >= 0) {
            return def->door_right;
        }
        return -1;
    }
    if (facing < 0 && def->door_left >= 0) {
        return def->door_left;
    }
    if (facing > 0 && def->door_right >= 0) {
        return def->door_right;
    }
    return -1;
}

void world_draw(const World *w, SDL_Renderer *r, SDL_Texture *tiles_tex) {
    for (int y = 0; y < ROOM_H; ++y) {
        for (int x = 0; x < ROOM_W; ++x) {
            TileType t = w->tiles[y][x];
            int frame = -1;
            switch (t) {
            case TILE_SOLID:
                frame = 0;
                break;
            case TILE_DOOR:
                frame = 1;
                break;
            case TILE_SPIKE:
                frame = 2;
                break;
            case TILE_TUNNEL:
                frame = 3;
                break;
            case TILE_EXIT:
                frame = 4;
                break;
            case TILE_DECO:
                frame = 5;
                break;
            default:
                break;
            }
            if (frame < 0) {
                continue;
            }
            SDL_Rect src = {frame * TILE, 0, TILE, TILE};
            SDL_Rect dst = {x * TILE, y * TILE, TILE, TILE};
            SDL_RenderCopy(r, tiles_tex, &src, &dst);
        }
    }
}
