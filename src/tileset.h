#ifndef _TILESET_H_
#define _TILESET_H_

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "settings.h"
#include "util.h"
#include "parse.h"
#include "pal.h"



struct tile_t {
    /* tiles are owned by a tileset, which store the width & height */
    int *data;
};

struct tileset_t {
    const char *name;

    /* filename from which this was loaded */
    const char *fname;

    /* tileset owns its tiles */
    int len;
    int tile_w;
    int tile_h;
    struct tile_t *tiles;
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
    repr_intmap(tile->data, tile_w, tile_h, "%s", "%X", depth+1);
}

int tile_parse(struct tile_t *tile, char **fdata, int tile_w, int tile_h){
    char *key = NULL;
    char *val = NULL;
    int key_len = 0;
    int val_len = 0;
    while(1){
        RET_IF_NZ(parse_item(fdata, &key, &key_len, &val, &val_len));
        if(key_len == 0){
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

struct tileset_t *tileset_create(const char *name, const char *fname, int tile_w, int tile_h, int len){
    struct tileset_t *tileset = malloc(sizeof(*tileset));
    LOG(); printf("Creating tileset: %p, name=%s, fname=%s, tile_w=%i, tile_h=%i, len=%i\n", tileset, name, fname, tile_w, tile_h, len);
    if(tileset == NULL)return NULL;
    tileset->name = name;
    tileset->fname = fname;
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
        if(key_len == 0){
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

    struct tileset_t *tileset = tileset_create(name, fname, tile_w, tile_h, len);
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


#endif