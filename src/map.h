#ifndef _MAP_H_
#define _MAP_H_

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "settings.h"
#include "util.h"
#include "parse.h"



struct tile_t {
    /* tiles are owned by a tileset, which store the width & height */
    int *data;
};

struct tileset_t {
    const char *name;

    /* tileset owns its tiles */
    int len;
    int tile_w;
    int tile_h;
    struct tile_t *tiles;
};

struct pal_t {
    const char *name;

    /* palette owns its colors */
    int len;
    SDL_Color *colors;
};

struct room_t {
    const char *name;
    struct tileset_t *tileset;
    struct pal_t *pal;

    /* room's data is a 2d array of indices into the tileset */
    int w;
    int h;
    int *data;
};


/********
 * TILE *
 ********/

int tile_init(struct tile_t *tile, int tile_w, int tile_h){
    int size = tile_w * tile_h;
    LOG(); printf("Initializing tile: %p, tile_w=%i, tile_h=%i\n", tile, tile_w, tile_h);
    tile->data = malloc(sizeof(*tile->data) * size);
    if(tile->data == NULL)return 1;
    for(int i = 0; i < size; i++)tile->data[i] = -1;
    return 0;
}

void tile_repr(struct tile_t *tile, int tile_w, int tile_h, int depth){
    if(DEBUG_REPR >= 1){
        LOG(); printf("Dumping tile: %p\n", tile);
    }
    REPR_FIELD_MULTI(data, depth)
    for(int i = 0; i < tile_h; i++){
        print_tabs(depth + 1);
        for(int j = 0; j < tile_w; j++){
            int c = tile->data[i * tile_w + j];
            if(c == -1)printf(".");
            else printf("%X", c);
            printf(" ");
        }
        printf("\n");
    }
}

int tile_parse(struct tile_t *tile, char **fdata, int tile_w, int tile_h){
    char *key = NULL;
    char *val = NULL;
    int key_len = 0;
    int val_len = 0;
    while(1){
        RET_IF_NZ(parse_item(fdata, &key, &key_len, &val, &val_len));
        if(key == NULL){
            LOG(); printf("Parse error: expected key \"data\"\n");
            return 2;
        }
        if(strncmp(key, "data", key_len) == 0){
            break;
        }else{
            LOG(); printf("Parse error: unexpected key \"%.*s\"\n", key_len, key);
            return 2;
        }
    }

    RET_IF_NZ(parse_intmap(fdata, tile->data, tile_w, tile_h, 16));

    if(DEBUG_LOAD >= 1){
        LOG(); printf("Parsed tile: %p\n", tile);
        tile_repr(tile, tile_w, tile_h, 1);
    }

    return 0;
}

int tile_render(struct tile_t *tile, int tile_w, int tile_h, int tile_x, int tile_y, struct pal_t *pal, SDL_Renderer *renderer){
    SDL_Rect rect;
    rect.w = TILE_PIXEL_W;
    rect.h = TILE_PIXEL_H;

    rect.y = tile_y;
    for(int i = 0; i < tile_h; i++){
        rect.x = tile_x;
        for(int j = 0; j < tile_w; j++){

            int color_i = tile->data[i * tile_w + j];
            if(color_i >= 0){
                SDL_Color *c = &pal->colors[color_i];
                RET_IF_SDL_ERR(SDL_SetRenderDrawColor(renderer, c->r, c->g, c->b, SDL_ALPHA_OPAQUE));
                RET_IF_SDL_ERR(SDL_RenderFillRect(renderer, &rect));
            }

            rect.x += rect.w;
        }
        rect.y += rect.h;
    }
    return 0;
}


/***********
 * TILESET *
 ***********/

struct tileset_t *tileset_create(const char *name, int tile_w, int tile_h, int len){
    struct tileset_t *tileset = malloc(sizeof(*tileset));
    LOG(); printf("Creating tileset: %p, name=%s, tile_w=%i, tile_h=%i, len=%i\n", tileset, name, tile_w, tile_h, len);
    if(tileset == NULL)return NULL;
    tileset->name = name;
    tileset->tile_w = tile_w;
    tileset->tile_h = tile_h;
    tileset->len = len;
    tileset->tiles = len == 0? NULL: malloc(sizeof(*tileset->tiles) * len);
    if(len != 0 && tileset->tiles == NULL)return NULL;
    for(int i = 0; i < len; i++){
        RET_NULL_IF_NZ(tile_init(&tileset->tiles[i], tile_w, tile_h));
    }
    return tileset;
}

void tileset_repr(struct tileset_t *tileset, int depth){
    if(DEBUG_REPR >= 1){
        LOG(); printf("Dumping tileset: %p\n", tileset);
    }
    REPR_FIELD(tileset, name, "%s", depth)
    REPR_FIELD(tileset, tile_w, "%i", depth)
    REPR_FIELD(tileset, tile_h, "%i", depth)
    REPR_FIELD(tileset, len, "%i", depth)
    REPR_FIELD_MULTI(tiles, depth)
    for(int i = 0; i < tileset->len; i++){
        tile_repr(&tileset->tiles[i], tileset->tile_w, tileset->tile_h, depth + 1);
    }
}

struct tileset_t *tileset_load(const char *fname){
    LOG(); printf("Loading tileset: fname=%s\n", fname);
    char *fdata = load_file(fname);
    if(fdata == NULL)return NULL;

    const char *name = "";
    int tile_w = 0;
    int tile_h = 0;
    int len = 0;

    char *key = NULL;
    char *val = NULL;
    int key_len = 0;
    int val_len = 0;
    while(1){
        RET_NULL_IF_NZ(parse_item(&fdata, &key, &key_len, &val, &val_len));
        if(key == NULL){
            LOG(); printf("Parse error: expected key \"tiles\"\n");
            return NULL;
        }
        if(strncmp(key, "name", key_len) == 0){
            name = strndup(val, val_len);
        }else if(strncmp(key, "tile_w", key_len) == 0){
            tile_w = atoi(val);
        }else if(strncmp(key, "tile_h", key_len) == 0){
            tile_h = atoi(val);
        }else if(strncmp(key, "len", key_len) == 0){
            len = atoi(val);
        }else if(strncmp(key, "tiles", key_len) == 0){
            break;
        }else{
            LOG(); printf("Parse error: unexpected key \"%.*s\"\n", key_len, key);
            return NULL;
        }
    }

    struct tileset_t *tileset = tileset_create(name, tile_w, tile_h, len);
    if(tileset == NULL)return NULL;

    for(int i = 0; i < len; i++){
        RET_NULL_IF_NZ(tile_parse(&tileset->tiles[i], &fdata, tile_w, tile_h));
    }

    if(DEBUG_LOAD >= 1){
        LOG(); printf("Loaded tileset: %p\n", tileset);
        tileset_repr(tileset, 1);
    }

    return tileset;
}


/***********
 * PALETTE *
 ***********/

void pal_color_init(SDL_Color *c, Uint8 r, Uint8 g, Uint8 b){
    c->r = r;
    c->g = g;
    c->b = b;
    c->a = 255;
}

void pal_color_repr(SDL_Color *c, int depth){
    print_tabs(depth);
    printf("%3i %3i %3i\n", c->r, c->g, c->b);
}

struct pal_t *pal_create(const char *name, int len){
    struct pal_t *pal = malloc(sizeof(*pal));
    LOG(); printf("Creating pal: %p, name=%s, len=%i\n", pal, name, len);
    if(pal == NULL)return NULL;
    pal->name = name;
    pal->len = len;
    pal->colors = len == 0? NULL: malloc(sizeof(*pal->colors) * len);
    if(len != 0 && pal->colors == NULL)return NULL;
    for(int i = 0; i < len; i++){
        pal_color_init(&pal->colors[i], 0, 0, 0);
    }
    return pal;
}

void pal_repr(struct pal_t *pal, int depth){
    if(DEBUG_REPR >= 1){
        LOG(); printf("Dumping pal: %p\n", pal);
    }
    REPR_FIELD(pal, name, "%s", depth)
    REPR_FIELD(pal, len, "%i", depth)
    REPR_FIELD_MULTI(colors, depth)
    for(int i = 0; i < pal->len; i++){
        pal_color_repr(&pal->colors[i], depth + 1);
    }
}

struct pal_t *pal_load(const char *fname){
    LOG(); printf("Loading pal: fname=%s\n", fname);
    char *fdata = load_file(fname);
    if(fdata == NULL)return NULL;

    const char *name = "";
    int len = 0;

    char *key = NULL;
    char *val = NULL;
    int key_len = 0;
    int val_len = 0;
    while(1){
        RET_NULL_IF_NZ(parse_item(&fdata, &key, &key_len, &val, &val_len));
        if(key == NULL){
            LOG(); printf("Parse error: expected key \"colors\"\n");
            return NULL;
        }
        if(strncmp(key, "name", key_len) == 0){
            name = strndup(val, val_len);
        }else if(strncmp(key, "len", key_len) == 0){
            len = atoi(val);
        }else if(strncmp(key, "colors", key_len) == 0){
            break;
        }else{
            LOG(); printf("Parse error: unexpected key \"%.*s\"\n", key_len, key);
            return NULL;
        }
    }

    struct pal_t *pal = pal_create(name, len);
    if(pal == NULL)return NULL;

    int *data = malloc(sizeof(*data) * len * 3);
    if(data == NULL)return NULL;
    RET_NULL_IF_NZ(parse_intmap(&fdata, data, 3, len, 10));
    for(int i = 0; i < len; i++){
        int i3 = i * 3;
        pal_color_init(&pal->colors[i], data[i3 + 0], data[i3 + 1], data[i3 + 2]);
    }

    if(DEBUG_LOAD >= 1){
        LOG(); printf("Loaded pal: %p\n", pal);
        pal_repr(pal, 1);
    }

    return pal;
}


/*******
 * MAP *
 *******/

struct room_t *room_create(const char *name, struct tileset_t *tileset, struct pal_t *pal, int w, int h){
    int size = w * h;
    struct room_t *room = malloc(sizeof(*room));
    LOG(); printf("Creating room: %p, name=%s, tileset=%p, pal=%p, w=%i, h=%i\n", room, name, tileset, pal, w, h);
    if(room == NULL)return room;
    room->name = name;
    room->tileset = tileset;
    room->pal = pal;
    room->w = w;
    room->h = h;
    room->data = size == 0? NULL: malloc(sizeof(*room->data) * size);
    if(size != 0 && room->data == NULL)return NULL;
    for(int i = 0; i < size; i++)room->data[i] = -1;
    return room;
}

void room_repr(struct room_t *room, int depth){
    if(DEBUG_REPR >= 1){
        LOG(); printf("Dumping room: %p\n", room);
    }
    REPR_FIELD(room, name, "%s", depth)
    REPR_FIELD_MULTI(tileset, depth)
    tileset_repr(room->tileset, depth + 1);
    REPR_FIELD_MULTI(pal, depth)
    pal_repr(room->pal, depth + 1);
    REPR_FIELD(room, w, "%i", depth)
    REPR_FIELD(room, h, "%i", depth)
    REPR_FIELD_MULTI(data, depth)
    int w = room->w;
    int h = room->h;
    for(int i = 0; i < h; i++){
        print_tabs(depth + 1);
        for(int j = 0; j < w; j++){
            int tile_i = room->data[i * w + j];
            if(tile_i == -1)printf("%3s", ".");
            else printf("%3i", tile_i);
            printf(" ");
        }
        printf("\n");
    }
}


struct room_t *room_load(const char *fname){
    LOG(); printf("Loading room: fname=%s\n", fname);
    char *fdata = load_file(fname);
    if(fdata == NULL)return NULL;

    const char *name = NULL;
    const char *tileset_fname = NULL;
    struct tileset_t *tileset = NULL;
    const char *pal_fname = NULL;
    struct pal_t *pal = NULL;
    int w = 0;
    int h = 0;

    char *key = NULL;
    char *val = NULL;
    int key_len = 0;
    int val_len = 0;
    while(1){
        RET_NULL_IF_NZ(parse_item(&fdata, &key, &key_len, &val, &val_len));
        if(key == NULL){
            LOG(); printf("Parse error: expected key \"data\"\n");
            return NULL;
        }
        if(strncmp(key, "name", key_len) == 0){
            name = strndup(val, val_len);
        }else if(strncmp(key, "tileset", key_len) == 0){
            tileset_fname = strndup(val, val_len);
            tileset = tileset_load(tileset_fname);
            if(tileset == NULL)return NULL;
        }else if(strncmp(key, "palette", key_len) == 0){
            pal_fname = strndup(val, val_len);
            pal = pal_load(pal_fname);
            if(pal == NULL)return NULL;
        }else if(strncmp(key, "w", key_len) == 0){
            w = atoi(val);
        }else if(strncmp(key, "h", key_len) == 0){
            h = atoi(val);
        }else if(strncmp(key, "data", key_len) == 0){
            break;
        }else{
            LOG(); printf("Parse error: unexpected key \"%.*s\"\n", key_len, key);
            return NULL;
        }
    }

    struct room_t *room = room_create(name, tileset, pal, w, h);
    if(room == NULL)return NULL;

    RET_NULL_IF_NZ(parse_intmap(&fdata, room->data, w, h, 16));

    if(DEBUG_LOAD >= 1){
        LOG(); printf("Loaded room: %p\n", room);
        room_repr(room, 1);
    }

    return room;
}


int room_render(struct room_t *room, int room_x, int room_y, SDL_Renderer *renderer){
    if(DEBUG_RENDER >= 1){
        LOG(); printf("Rendering room: %p\n", room);
    }

    struct tileset_t *tileset = room->tileset;
    struct pal_t *pal = room->pal;

    int w = room->w;
    int h = room->h;
    int tile_w = tileset->tile_w;
    int tile_h = tileset->tile_h;

    int tile_y = room_y;
    for(int i = 0; i < h; i++){
        int tile_x = room_x;
        for(int j = 0; j < w; j++){

            int tile_i = room->data[i * w + j];
            if(tile_i >= 0){
                struct tile_t *tile = &tileset->tiles[tile_i];
                RET_IF_NZ(tile_render(tile, tile_w, tile_h, tile_x, tile_y, pal, renderer));
            }

            tile_x += tile_w * TILE_PIXEL_W;
        }
        tile_y += tile_h * TILE_PIXEL_H;
    }
    return 0;
}



#endif