#ifndef _SETTINGS_H_
#define _SETTINGS_H_


/* w, h of tiles in pixels */
#define TILE_W 16
#define TILE_H 16

/* w, h of view in tiles */
#define VIEW_W 32
#define VIEW_H 32

/* w, h of screen / window */
#define SCW (TILE_W * VIEW_W)
#define SCH (TILE_H * VIEW_H)

/* debug levels */
#define DEBUG_REPR 0
#define DEBUG_PARSE 0
#define DEBUG_LOAD 0

#endif