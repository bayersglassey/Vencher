#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "map.h"
#include "tileset.h"
#include "parse.h"


enum keys_e {
    KEY_U, KEY_D, KEY_L, KEY_R,
    KEY_DROP,
    KEYS
};

struct controller_t {
    /* Sprite controlled by this controller */
    struct sprite_t *sprite;

    /* Key info */
    SDL_Keycode keycodes[KEYS];
    bool key_was_down[KEYS];
    bool key_is_down[KEYS];

    /* Is this a CPU player? (AI-controlled) */
    bool is_cpu;
};

struct sprite_t {
    const char *name;

    /* filename from which this was loaded */
    const char *fname;

    /* map coords of room we're in */
    int room_x;
    int room_y;

    /* coords within room */
    int x;
    int y;

    struct tileset_t *tileset;

    /* index of tile within tileset */
    int frame;

    struct controller_t controller;
};



/**************
 * CONTROLLER *
 **************/

void controller_init(struct controller_t *controller, struct sprite_t *sprite, bool is_cpu){
    LOG(); printf("Initializing controller: %p, sprite=%p, is_cpu=%i\n", controller, sprite, is_cpu);
    controller->sprite = sprite;
    controller->is_cpu = is_cpu;
    for(int i = 0; i < KEYS; i++){

        /* The escape key is not valid as a controller key code.
        Only because there's no SDLK_NONE... or SDLK_UNSET... */
        controller->keycodes[i] = SDLK_ESCAPE;

        controller->key_was_down[i] = false;
        controller->key_is_down[i] = false;
    }
}

void controller_set_keycodes(struct controller_t *controller, SDL_Keycode keycodes[KEYS]){
    for(int i = 0; i < KEYS; i++)controller->keycodes[i] = keycodes[i];
}

void controller_repr(struct controller_t *controller, int depth){
    if(DEBUG_REPR >= 1){
        LOG(); printf("Dumping controller: %p\n", controller);
    }

    REPR_FIELD_MULTI(keycodes, depth)
    for(int i = 0; i < KEYS; i++){
        print_tabs(depth + 1);
        printf("%i\n", controller->keycodes[i]);
    }

    REPR_FIELD_MULTI(key_is_down, depth)
    for(int i = 0; i < KEYS; i++){
        print_tabs(depth + 1);
        printf("%i\n", controller->key_is_down[i]);
    }

    REPR_FIELD_MULTI(key_was_down, depth)
    for(int i = 0; i < KEYS; i++){
        print_tabs(depth + 1);
        printf("%i\n", controller->key_was_down[i]);
    }

    REPR_FIELD(controller, is_cpu, "%i", depth)
}


/**********
 * SPRITE *
 **********/

struct sprite_t *sprite_create(const char *name, const char *fname, struct tileset_t *tileset, int room_x, int room_y, int x, int y, bool is_cpu){
    struct sprite_t *sprite = malloc(sizeof(*sprite));
    LOG(); printf("Creating sprite: %p, name=%s, fname=%s, tileset=%p, room_x=%i, room_y=%i, x=%i, y=%i, is_cpu=%i\n",
        sprite, name, fname, tileset, room_x, room_y, x, y, is_cpu);
    if(sprite == NULL)return NULL;
    sprite->name = name;
    sprite->fname = fname;
    sprite->tileset = tileset;
    sprite->room_x = room_x;
    sprite->room_y = room_y;
    sprite->x = x;
    sprite->y = y;
    sprite->frame = 0;
    controller_init(&sprite->controller, sprite, is_cpu);
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
    REPR_FIELD(sprite, x, "%i", depth)
    REPR_FIELD(sprite, y, "%i", depth)
    REPR_FIELD(sprite, frame, "%i", depth)
}

struct sprite_t *sprite_load(const char *fname, int room_x, int room_y, int x, int y, bool is_cpu){
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

    struct sprite_t *sprite = sprite_create(name, fname, tileset, room_x, room_y, x, y, is_cpu);
    if(sprite == NULL)return NULL;

    if(DEBUG_LOAD >= 1){
        LOG(); printf("Loaded sprite: %p\n", sprite);
        sprite_repr(sprite, 1);
    }

    return sprite;
}

int sprite_render(struct sprite_t *sprite, int world_x, int world_y, struct pal_t *pal, SDL_Renderer *renderer){
    if(DEBUG_RENDER >= 1){
        LOG(); printf("Rendering sprite: %p\n", sprite);
    }

    struct tileset_t *tileset = sprite->tileset;
    struct tile_t *tile = &tileset->tiles[sprite->frame];
    RET_IF_NZ(tile_render(tile, tileset->tile_w, tileset->tile_h, world_x + sprite->x, world_y + sprite->y, pal, renderer));


    return 0;
}



#endif