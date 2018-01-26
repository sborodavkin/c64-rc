/******************************************************************************/
/* Ray casting engine for Commodore 64.                                       */
/* (C) 2018, Sergey Borodavkin                                                */
/* sergei.borodavkin@gmail.com                                                */
/*                                                                            */
/* raycaster.h                                                                */
/* Generic declarations                                                       */
/*                                                                            */
/******************************************************************************/

#ifndef RC_H
#define RC_H

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Common declarations.
////////////////////////////////////////////////////////////////////////////////

// Debug flag. Enable to print debugging info instead of rendering.
//#define DEBUG 0

// Map dimensions in map coordinates.
#define MAP_WIDTH 24
#define MAP_HEIGHT 24

// World map as 2D array. 0 is empty square, >0 are different wall types.
extern uint8_t worldMap[][MAP_WIDTH];

// Screen resolution we operate in.
#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 25

// Field of view in brad (0-255). 42 corresponds to 60 degrees.
#define FOV 42

////////////////////////////////////////////////////////////////////////////////
// Textures.
////////////////////////////////////////////////////////////////////////////////

// Texture resolution (width = height).
#define TEXTURE_SIZE 32

// For texture mapping we use fixed point calculation with 10 bits for the
// fractional part. This is max value of the fixed part (=32) shifted left
// by 10.
#define TEXTURE_SIZE_FIXED_PART 32768u

// First texture. Textures are stored in vertical stripes, i.e.
// [x*TEXTURE_SIZE+y] containing 0 for background color and 1 for foreground.
extern uint8_t textureBrick[];
// Color map of texture. Values 0-1 are background and foreground color
// that are rendered when we look at SIDE_HOR; values 2-3 are background
// and foreground color that are rendered when we look at SIDE_VER.
extern uint8_t textureBrickColors[];
extern uint8_t textureSquare[];
extern uint8_t textureSquareColors[];

// TEXTURE_SIZE_FIXED_PART divided by all possible values of line height
// (1..SCREEN_HEIGHT)
extern uint16_t textureScaleMap[];

////////////////////////////////////////////////////////////////////////////////
// Wall sides operations.
////////////////////////////////////////////////////////////////////////////////

// Which side of wall did the DDA algorithm hit: horizontal, vertical or
// undefined.
#define SIDE_HOR 0
#define SIDE_VER 1
#define SIDE_UNDEF 2

// This is used to point DDA which side of wall to choose. I.e. when we say
// that SIDE_HOR is TOO_FAR, then DDA assumes intersection with SIDE_VER.
// This has to be "far" enough not to confuse the algorithm.
#define TOO_FAR 1024

// Visible and hidden sides of walls.
// sidesMap[y][x] contains a 8-bit mask DDCCBBAA, where
// AA = 00 if northern side of wall is hidden,
// BB = 00 if eastern side of wall is hidden,
// CC = 00 if southern side of wall is hidden,
// DD = 00 if western side of wall is hidden.
// For visible sides, 11 is stored.
// Example: for wall marked with "*" here we store 00110011 = 0x33.
// 111
// 0*0
// 111
extern uint8_t sidesMap[][MAP_WIDTH];

////////////////////////////////////////////////////////////////////////////////
// Double buffering.
////////////////////////////////////////////////////////////////////////////////

// Back buffer for color map. Color RAM can't be switched like character buffer
// so we draw every frame to it and then copy to VIC color RAM.
extern uint8_t backColorBuf[];

////////////////////////////////////////////////////////////////////////////////
// Calculation helpers.
////////////////////////////////////////////////////////////////////////////////

#define NUM_ANGLES 256

// Cos table, 0-255 => -32..32
extern int8_t COS[];
  
// Sin table, 0-255 => -32..32
extern int8_t SIN[];

////////////////////////////////////////////////////////////////////////////////
// Functions.
////////////////////////////////////////////////////////////////////////////////

// KERNAL function GETIN - get a character from default input device.
uint8_t __fastcall__ (*cbm_k_getin)(void) = 0xFFE4;

// Waits for a keypress and returns the char.
uint8_t waitForKey();

// Init textures data.
void initTextures();

// Init wall sides map for the world map.
void compileMapSides();

// Flushes back- and color-buffer to video RAM.
void flushBuffer();

// Helper to calculate a distance between two points on a ray with given angle.
uint16_t distance(uint8_t angle, uint16_t ax, uint16_t ay,
                  uint16_t bx, uint16_t by);

#endif
