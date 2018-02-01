/******************************************************************************/
/* Ray casting engine for Commodore 64.                                       */
/* (C) 2018, Sergey Borodavkin                                                */
/* sergei.borodavkin@gmail.com                                                */
/*                                                                            */
/* world.c                                                                    */
/* World map implementation.                                                  */
/*                                                                            */
/******************************************************************************/

#include <stdint.h>
#include "world.h"


uint8_t worlds[NUM_WORLDS][WORLD_0_SIZE] = {
  {0, 0, 6, 0, 6, 8, 15, 8, 15, 15, 0, 15, 0, 0}
};

uint8_t wallTextures[NUM_WORLDS][WORLD_0_SIZE/2] = {
  {1, 1, 2, 2, 2, 1}
};

uint16_t wallDist[NUM_WORLDS][WORLD_0_SIZE/2];

uint16_t intersection[NUM_WORLDS][WORLD_0_SIZE/2];

uint16_t sides[NUM_WORLDS][WORLD_0_SIZE/2];

