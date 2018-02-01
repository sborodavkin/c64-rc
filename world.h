/******************************************************************************/
/* Ray casting engine for Commodore 64.                                       */
/* (C) 2018, Sergey Borodavkin                                                */
/* sergei.borodavkin@gmail.com                                                */
/*                                                                            */
/* world.h                                                                    */
/* World map.                                                                 */
/*                                                                            */
/******************************************************************************/

#ifndef WORLD_H
#define WORLD_H

#define NUM_WORLDS 1

#define CUR_WORLD 0

// Points * 2 plus end point which should be equal to start point.
#define WORLD_0_SIZE 14

extern uint8_t worlds[][WORLD_0_SIZE];
extern uint8_t wallTextures[][WORLD_0_SIZE/2];
extern uint16_t wallDist[][WORLD_0_SIZE/2];
extern uint16_t intersection[][WORLD_0_SIZE/2];
extern uint16_t sides[][WORLD_0_SIZE/2];

////////////////////////////////////////////////////////////////////////////////
// Functions.
////////////////////////////////////////////////////////////////////////////////

void initWorlds();

#endif
