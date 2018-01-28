#ifndef _WORLD_H_
#define _WORLD_H_

#include <stdlib.h>
#include <stdio.h>

#include "map.h"
#include "parse.h"
#include "sprite.h"


struct world_t {
    /* Represents the state of the entire game world: map, entities, etc. */

    struct map_t *map;

    int room_x;
    int room_y;
    struct room_t *room;

    int n_sprites;
    struct sprite_t **sprites;
};

struct world_t *world_create(struct map_t *map){
    struct world_t *world = malloc(sizeof(*world));
    LOG(); printf("Creating world: %p, map=%p\n", world, map);
    if(world == NULL)return NULL;

    world->map = map;
    world->room_x = map->entrance_x;
    world->room_y = map->entrance_y;

    world->room = map_get_room(map, world->room_x, world->room_y, false);
    if(world->room == NULL){
        LOG(); printf("Couldn't get initial room: room_x=%i, room_y=%i\n", world->room_x, world->room_y);
        return NULL;
    }

    world->n_sprites = 0;
    world->sprites = NULL;

    return world;
}

int world_sprites_resize(struct world_t *world, int new_n_sprites){
    int diff = new_n_sprites - world->n_sprites;
    for(int i = new_n_sprites; i < world->n_sprites; i++){
        if(world->sprites[i] != NULL){
            LOG(); printf("Tried to shrink sprite array past a non-NULL sprite: old_n_sprites=%i, new_n_sprites=%i\n",
                world->n_sprites, new_n_sprites);
            return 2;
        }
    }
    struct sprite_t **new_sprites = realloc(world->sprites, sizeof(*world->sprites) * new_n_sprites);
    if(new_sprites == NULL)return 1;
    for(int i = world->n_sprites; i < new_n_sprites; i++)new_sprites[i] = NULL;
    world->n_sprites = new_n_sprites;
    world->sprites = new_sprites;
    return 0;
}

int world_sprites_add(struct world_t *world, struct sprite_t *sprite){
    int sprite_i;
    for(sprite_i = 0; sprite_i < world->n_sprites; sprite_i++){
        if(world->sprites[sprite_i] == NULL)break;
    }
    if(sprite_i >= world->n_sprites){
        int new_n_sprites = world->n_sprites * 2;
        if(new_n_sprites == 0)new_n_sprites = 1;
        RET_IF_NZ(world_sprites_resize(world, new_n_sprites));
    }
    world->sprites[sprite_i] = sprite;
    return 0;
}

void world_repr(struct world_t *world, int depth){
    if(DEBUG_REPR >= 1){
        LOG(); printf("Dumping world: %p\n", world);
    }

    REPR_FIELD_MULTI(map, depth)
    map_repr(world->map, depth + 1);

    REPR_FIELD(world, room_x, "%i", depth)
    REPR_FIELD(world, room_y, "%i", depth)

    REPR_FIELD_MULTI(room, depth)
    room_repr(world->room, depth + 1);

    REPR_FIELD_MULTI(sprites, depth)
    for(int i = 0; i < world->n_sprites; i++){
        struct sprite_t *sprite = world->sprites[i];
        if(sprite == NULL){
            print_tabs(depth + 1);
            printf("NULL\n");
        }else{
            sprite_repr(sprite, depth + 1);
        }
    }
}

int world_render(struct world_t *world, int world_x, int world_y, SDL_Renderer *renderer){
    if(DEBUG_RENDER >= 1){
        LOG(); printf("Rendering world: %p\n", world);
    }

    RET_IF_NZ(room_render(world->room, world_x, world_y, renderer));

    return 0;
}



#endif