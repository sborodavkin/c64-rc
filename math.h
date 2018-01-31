/******************************************************************************/
/* Ray casting engine for Commodore 64.                                       */
/* (C) 2018, Sergey Borodavkin                                                */
/* sergei.borodavkin@gmail.com                                                */
/*                                                                            */
/* math.h                                                                     */
/* Math routines.                                                             */
/*                                                                            */
/******************************************************************************/

#ifndef MATH_H
#define MATH_H

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Common declarations.
////////////////////////////////////////////////////////////////////////////////

#define DIFF(a, b) ((a > b) ? a - b : b - a)

void castRay(uint8_t posX, uint8_t posY, uint8_t globalRayAngle,
             int8_t localAngle, uint8_t* totalDistance, uint8_t* side,
             uint8_t* mapValue, uint8_t* textureX);

void getWallSlice(uint8_t correctDist, uint8_t* drawStart, uint8_t* drawEnd);

// Helper to calculate a distance between two points on a ray with given angle.
uint8_t distance(uint8_t absCos, uint8_t absSin, uint8_t ax, uint8_t ay,
                 uint8_t bx, uint8_t by);

void getAyBx(uint8_t globalRayPosX, uint8_t globalRayPosY,
             int8_t globalRayDirX, int8_t globalRayDirY,
             uint8_t* ay, uint8_t* bx);

#endif