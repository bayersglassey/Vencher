
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "util.h"
#include "settings.h"
#include "world.h"
#include "map.h"
#include "sprite.h"


int mainloop(SDL_Renderer *renderer, int n_args, char *args[]){
    SDL_Event event;

    struct map_t *map = map_load("data/map0.txt");
    if(map == NULL)return 1;

    struct world_t *world = world_create(map);
    if(world == NULL)return 1;

    struct sprite_t *player = sprite_load("data/sprites/player.txt", world->room_x, world->room_y, 0, 0, false);
    if(player == NULL)return 1;

    SDL_Keycode keycodes[KEYS] = {
        SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
        SDLK_SPACE
    };
    controller_set_keycodes(&player->controller, keycodes);

    RET_IF_NZ(world_sprites_add(world, player));

    LOG(); printf("Using world:\n");
    world_repr(world, 1);

    bool loop = true;
    while(loop){

        /* START TIMER */
        Uint32 last_tick = SDL_GetTicks();
        Uint32 next_tick = last_tick + 30;

        /* RENDER WORLD */
        RET_IF_SDL_ERR(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE));
        RET_IF_SDL_ERR(SDL_RenderClear(renderer));
        RET_IF_NZ(world_render(world, 0, 0, renderer));
        SDL_RenderPresent(renderer);

        /* PREPARE WORLD FOR A NEW TICK */
        RET_IF_NZ(world_prepare_tick(world));

        /* HANDLE EVENTS */
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                loop = false;
            }else if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP){
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    if(event.type == SDL_KEYDOWN){
                        loop = false;
                    }
                }else{
                    /* UPDATE CONTROLLER KEY STATES */
                    for(int i = 0; i < world->n_sprites; i++){
                        struct sprite_t *sprite = world->sprites[i];
                            if(sprite != NULL){
                            struct controller_t *controller = &sprite->controller;
                            for(int j = 0; j < KEYS; j++){
                                if(controller->keycodes[i] == event.key.keysym.sym){
                                    if(event.type == SDL_KEYDOWN){
                                        controller->key_is_down[i] = true;
                                        controller->key_was_down[i] = true;
                                    }else if(event.type == SDL_KEYUP){
                                        controller->key_is_down[i] = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        /* DO WHATEVER A WORLD DOES DURING A TICK */
        RET_IF_NZ(world_do_tick(world));

        /* DELAY */
        Uint32 new_tick = SDL_GetTicks();
        if(new_tick < next_tick){
            Uint32 wait_ticks = next_tick - new_tick;
            SDL_Delay(wait_ticks);
        }

    }
    return 0;
}

int main(int n_args, char *args[]){
    int e = 0;
    if(SDL_Init(SDL_INIT_VIDEO)){
        e = 1;
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
    }else{
        SDL_Window *window = SDL_CreateWindow("VENCHER",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            SCW, SCH, SDL_WINDOW_SHOWN);

        if(!window){
            e = 1;
            fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        }else{
            SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

            if(!renderer){
                e = 1;
                fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
            }else{
                e = mainloop(renderer, n_args, args);
                printf("Destroying renderer\n");
                SDL_DestroyRenderer(renderer);
            }
            printf("Destroying window\n");
            SDL_DestroyWindow(window);
        }
        printf("Quitting SDL\n");
        SDL_Quit();
    }
    fprintf(stderr, "Exiting with code: %i\n", e);
    return e;
}
