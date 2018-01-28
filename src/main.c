
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

    struct sprite_t *player = sprite_load("data/sprites/player.txt", world->room_x, world->room_y);
    if(player == NULL)return 1;

    RET_IF_NZ(world_sprites_add(world, player));

    LOG(); printf("Using world:\n");
    world_repr(world, 1);

    int rot = 0;
    bool loop = true;
    bool refresh = true;
    bool keydown_shift = false;
    int keydown_l = 0;
    int keydown_r = 0;
    while(loop){
        if(refresh){
            refresh = false;
            RET_IF_SDL_ERR(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE));
            RET_IF_SDL_ERR(SDL_RenderClear(renderer));
            RET_IF_NZ(world_render(world, 0, 0, renderer));
            SDL_RenderPresent(renderer);
        }
        SDL_PollEvent(&event);
        switch(event.type){
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_ESCAPE)loop = false;
                if(event.key.keysym.sym == SDLK_0){rot = 0; refresh = true;}
                if(event.key.keysym.sym == SDLK_LEFT && keydown_l == 0)keydown_l = 2;
                if(event.key.keysym.sym == SDLK_RIGHT && keydown_r == 0)keydown_r = 2;
                if(event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT)keydown_shift = true;
                break;
            case SDL_KEYUP:
                if(event.key.keysym.sym == SDLK_LEFT)keydown_l = 0;
                if(event.key.keysym.sym == SDLK_RIGHT)keydown_r = 0;
                if(event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT)keydown_shift = false;
                break;
            case SDL_QUIT: loop = false; break;
            default: break;
        }
        if(keydown_l >= (keydown_shift? 2: 1)){rot += 1; refresh = true; keydown_l = 1;}
        if(keydown_r >= (keydown_shift? 2: 1)){rot -= 1; refresh = true; keydown_r = 1;}
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
