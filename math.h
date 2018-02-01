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

#define SIGN_POS 1
#define SIGN_NEG -1


#define DIFF(a, b) ((a > b) ? a - b : b - a)

uint16_t castRay(uint8_t posX, uint8_t posY, uint8_t globalRayAngle,
                int8_t localAngle, uint16_t* totalDistance, uint8_t* side,
                uint8_t* wallTexture, uint8_t* textureX);

void getWallSlice(uint8_t correctDist, uint8_t* drawStart, uint8_t* drawEnd);

void getAyBx(uint8_t globalRayPosX, uint8_t globalRayPosY,
             int8_t globalRayDirX, int8_t globalRayDirY,
             uint8_t* ay, uint8_t* bx);

#endif
