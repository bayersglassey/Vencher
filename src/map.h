#ifndef _MAP_H_
#define _MAP_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "settings.h"
#include "util.h"
#include "parse.h"



struct tile_t {
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

struct map_t {
    const char *name;

    /* map references a tileset */
    struct tileset_t *tileset;

    /* map's data is a 2d array of indices into the tileset */
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


/*******
 * MAP *
 *******/

struct map_t *map_create(const char *name, struct tileset_t *tileset, int w, int h){
    int size = w * h;
    struct map_t *map = malloc(sizeof(*map));
    LOG(); printf("Creating map: %p, name=%s, tileset=%p, w=%i, h=%i\n", map, name, tileset, w, h);
    if(map == NULL)return map;
    map->name = name;
    map->tileset = tileset;
    map->w = w;
    map->h = h;
    map->data = size == 0? NULL: malloc(sizeof(*map->data) * size);
    if(size != 0 && map->data == NULL)return NULL;
    for(int i = 0; i < size; i++)map->data[i] = -1;
    return map;
}

void map_repr(struct map_t *map, int depth){
    if(DEBUG_REPR >= 1){
        LOG(); printf("Dumping map: %p\n", map);
    }
    REPR_FIELD(map, name, "%s", depth)
    REPR_FIELD_MULTI(tileset, depth)
    tileset_repr(map->tileset, depth + 1);
    REPR_FIELD(map, w, "%i", depth)
    REPR_FIELD(map, h, "%i", depth)
    REPR_FIELD_MULTI(data, depth)
    int w = map->w;
    int h = map->h;
    for(int i = 0; i < h; i++){
        print_tabs(depth + 1);
        for(int j = 0; j < w; j++){
            int tile_i = map->data[i * w + j];
            if(tile_i == -1)printf("%3s", ".");
            else printf("%3i", tile_i);
            printf(" ");
        }
        printf("\n");
    }
}


struct map_t *map_load(const char *fname){
    LOG(); printf("Loading map: fname=%s\n", fname);
    char *fdata = load_file(fname);
    if(fdata == NULL)return NULL;

    const char *name = NULL;
    const char *tileset_fname = NULL;
    struct tileset_t *tileset = NULL;
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

    struct map_t *map = map_create(name, tileset, w, h);
    if(map == NULL)return NULL;

    RET_NULL_IF_NZ(parse_intmap(&fdata, map->data, w, h, 16));

    if(DEBUG_LOAD >= 1){
        LOG(); printf("Loaded map: %p\n", map);
        map_repr(map, 1);
    }

    return map;
}



#endif