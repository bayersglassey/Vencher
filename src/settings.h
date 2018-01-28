#ifndef _SETTINGS_H_
#define _SETTINGS_H_


/* w, h in actual pixels of the "pixels" in tile data */
#define TILE_PIXEL_W 4
#define TILE_PIXEL_H 4

/* w, h of view in tile pixels */
#define VIEW_W 64
#define VIEW_H 64

/* w, h of screen / window in actual pixels */
#define SCW (TILE_PIXEL_W * VIEW_W)
#define SCH (TILE_PIXEL_H * VIEW_H)

/* debug levels */
#define DEBUG_REPR 0
#define DEBUG_PARSE 0
#define DEBUG_LOAD 0
#define DEBUG_RENDER 1

#endif