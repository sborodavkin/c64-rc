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
#define MAP_WIDTH 16
#define MAP_HEIGHT 16

// Size of a map unit (i.e. how many player coordinates are in a map coord.)
#define MAP_UNIT_SIZE 16
// Power of 2 factor of MAP_UNIT_SIZE. If MAP_UNIT_SIZE changes to 32, this
// should be changed to 5 and so on.
#define MAP_UNIT_POWER 4

// Screen resolution we operate in.
#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 25

// Field of view in brad (0-255). 42 corresponds to 60 degrees.
#define FOV 42

// String buffer to print info line.
extern char infoStrBuf[];

////////////////////////////////////////////////////////////////////////////////
// Textures.
////////////////////////////////////////////////////////////////////////////////

// Texture resolution (width = height).
#define TEXTURE_SIZE MAP_UNIT_SIZE

// For texture mapping we use fixed point calculation with 4 bits for the
// fractional part. This is max value of the fixed part shifted left
// by 4.
#define TEXTURE_SIZE_FIXED_PART 0xF0

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
extern uint8_t textureScaleMap[];

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
#define TOO_FAR 255

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

// Cos table, 0-255 => -16..16
extern int8_t COS[];
  
// Sin table, 0-255 => -16..16
extern int8_t SIN[];

extern int16_t USTAR_COS[];
extern int16_t USTAR_SIN[];
#define USTAR_SCALE 7

////////////////////////////////////////////////////////////////////////////////
// Functions.
////////////////////////////////////////////////////////////////////////////////

// KERNAL function GETIN - get a character from default input device.
extern uint8_t __fastcall__ (*cbm_k_getin)(void);

// Waits for a keypress and returns the char.
uint8_t waitForKey();

// Init textures data.
void initTextures();

// Init U*.
void initUStar();

// Flushes back- and color-buffer to video RAM.
void flushBuffer();

/* Prints the info line.
 *
 * Args:
 *   posX: global x-position of player.
 *   posY: global y-position of player.
 *   angle: global camera angle.
 *   clkFps: clock_t ticks per frame.
 */
void printInfo(uint8_t posX, uint8_t posY, uint8_t angle,
               uint16_t clkFps);

#endif
