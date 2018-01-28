#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdlib.h>
#include <stdio.h>

#include "map.h"
#include "tileset.h"
#include "parse.h"


struct sprite_t {
    const char *name;

    /* filename from which this was loaded */
    const char *fname;

    int room_x;
    int room_y;

    struct tileset_t *tileset;
};


struct sprite_t *sprite_create(const char *name, const char *fname, struct tileset_t *tileset, int room_x, int room_y){
    struct sprite_t *sprite = malloc(sizeof(*sprite));
    LOG(); printf("Creating sprite: %p, name=%s, fname=%s, tileset=%p, room_x=%i, room_y=%i\n", sprite, name, fname, tileset, room_x, room_y);
    if(sprite == NULL)return NULL;
    sprite->name = name;
    sprite->fname = fname;
    sprite->tileset = tileset;
    sprite->room_x = room_x;
    sprite->room_y = room_y;
    return sprite;
}

void sprite_repr(struct sprite_t *sprite, int depth){
    if(DEBUG_REPR >= 1){
        LOG(); printf("Dumping sprite: %p\n", sprite);
    }
    REPR_FIELD(sprite, name, "%s", depth)
    REPR_FIELD_EXT(sprite, tileset, tileset->fname, "%s", depth)
    REPR_FIELD(sprite, room_x, "%i", depth)
    REPR_FIELD(sprite, room_y, "%i", depth)
}

struct sprite_t *sprite_load(const char *fname, int room_x, int room_y){
    LOG(); printf("Loading sprite: fname=%s\n", fname);
    char *fdata = load_file(fname);
    if(fdata == NULL)return NULL;

    const char *name = "";
    struct tileset_t *tileset = NULL;
    const char *tileset_fname = NULL;

    char *key = NULL;
    char *val = NULL;
    int key_len = 0;
    int val_len = 0;
    while(1){
        RET_NULL_IF_NZ(parse_item(&fdata, &key, &key_len, &val, &val_len));
        if(key_len == 0)break;
        if(strncmp(key, "name", key_len) == 0){
            name = strndup(val, val_len);
        }else if(strncmp(key, "tileset", key_len) == 0){
            tileset_fname = strndup(val, val_len);
            tileset = tileset_load(tileset_fname);
            if(tileset == NULL)return NULL;
        }else{
            LOG(); printf("Parse error: unexpected key \"%.*s\"\n", key_len, key);
            return NULL;
        }
    }

    struct sprite_t *sprite = sprite_create(name, fname, tileset, room_x, room_y);
    if(sprite == NULL)return NULL;

    if(DEBUG_LOAD >= 1){
        LOG(); printf("Loaded sprite: %p\n", sprite);
        sprite_repr(sprite, 1);
    }

    return sprite;
}


#endif