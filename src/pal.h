#ifndef _PAL_H_
#define _PAL_H_

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "settings.h"
#include "util.h"
#include "parse.h"


struct pal_t {
    const char *name;

    /* filename from which this was loaded */
    const char *fname;

    /* palette owns its colors */
    int len;
    SDL_Color *colors;
};




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

struct pal_t *pal_create(const char *name, const char *fname, int len){
    struct pal_t *pal = malloc(sizeof(*pal));
    LOG(); printf("Creating pal: %p, name=%s, fname=%s, len=%i\n", pal, name, fname, len);
    if(pal == NULL)return NULL;
    pal->name = name;
    pal->fname = fname;
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
        if(key_len == 0){
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

    struct pal_t *pal = pal_create(name, fname, len);
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


#endif