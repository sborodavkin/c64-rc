/******************************************************************************/
/* Ray casting engine for Commodore 64.                                       */
/* (C) 2018, Sergey Borodavkin                                                */
/* sergei.borodavkin@gmail.com                                                */
/*                                                                            */
/* math.c                                                                     */
/* Math routines implementation.                                              */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include "raycaster.h"
#include "world.h"


#define POINT_IN_RANGE_AND_INCREMENT(p, c1, c2) ((p++ - c1) < (c2-c1))

uint8_t getRayToLineSegmentIntersection(uint8_t posX, uint8_t posY,
    int8_t globalRayDirX, int8_t globalRayDirY,
    uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t* intersectionPointOnWall)
{
  uint8_t minWallCoord, maxWallCoord;  
  int16_t t, intersection;

  if (x1 == x2) { 
    // Vertical line.
    if (globalRayDirX == 0) {
      // Ray parallel to wall, no intersection.
      *intersectionPointOnWall = 0;
      return 0;
    }
    t = (int16_t)(x1 - posX) / globalRayDirX;
    if (t < 0) {
      // Different signs, no intersection.
      *intersectionPointOnWall = 0;
      return 0;
    }
    if (y1 < y2) {
      minWallCoord = y1;
      maxWallCoord = y2;
    } else {
      minWallCoord = y2;
      maxWallCoord = y1;
    }
    // Check if other coord is within line segment.
    intersection = (int16_t)posY + (t * globalRayDirY);
    if (POINT_IN_RANGE_AND_INCREMENT(intersection, minWallCoord, maxWallCoord)) {
      *intersectionPointOnWall = intersection;
      return t;
    }
  } else {
    // Horizontal line.
    if (globalRayDirY == 0) {
      // Ray parallel to wall, no intersection.
      *intersectionPointOnWall = 0;
      return 0;
    }
    t = (int16_t)(y1 - posY) / globalRayDirY;
    if (t < 0) {
      // Different signs, no intersection.
      *intersectionPointOnWall = 0;
      return 0;
    }
    if (x1 < x2) {
      minWallCoord = x1;
      maxWallCoord = x2;
    } else {
      minWallCoord = x2;
      maxWallCoord = x1;
    }
    // Check if other coord is within line segment.
    intersection = (int16_t)posX + (t * globalRayDirX);
    if (POINT_IN_RANGE_AND_INCREMENT(intersection, minWallCoord, maxWallCoord)) {
      *intersectionPointOnWall = intersection;
      return t;
    }
  }
}


uint16_t correctDist(int16_t rayDist, uint8_t angle, int16_t rayDirX, int16_t rayDirY) {
  uint16_t distX = abs(rayDirX*rayDist), distY = abs(rayDirY*rayDist);
  if (distX > distY) {
    // X is more distinct, use cos.v
#ifdef DEBUG
    printf("\r\nxdiv %D by %D", (distX << 4), abs((int16_t)COS[angle]));
#endif      
    return (distX << 4) / abs((int16_t)COS[angle]);
  } else {
#ifdef DEBUG
    printf("\r\nydiv %D by %D", (distY << 4), abs((int16_t)SIN[angle]));
#endif  
    // Y is more distinct, use sin.
    return (distY << 4) / abs((int16_t)SIN[angle]);
  }
}


uint16_t castRay(uint8_t posX, uint8_t posY, uint8_t globalRayAngle,
    int8_t localAngle, uint16_t* totalDistance, uint8_t* side,
    uint8_t* wallTexture, uint8_t* textureX) {
  uint8_t angle;
  // Global direction of current ray
  int8_t globalRayDirX = COS[globalRayAngle],
         globalRayDirY = -SIN[globalRayAngle];  // Inverse as Y axis goes down.
  uint8_t wallIt = 0;  // Wall iterator in a current world.
  int16_t rayDist;
  uint8_t w1X, w1Y, w2X, w2Y;
  uint8_t found = 0, wallIdx = 0;
  uint8_t i = 0;
  uint16_t intWall = 0;

#ifdef DEBUG
    printf("\r\nind:px=%D,py=%D,dx=%D,dy=%D",
      posX, posY, globalRayDirX, globalRayDirY);
#endif
  
  while (wallIt < WORLD_0_SIZE-2) {
    w1X = worlds[CUR_WORLD][wallIt] << MAP_UNIT_POWER;
    w1Y = worlds[CUR_WORLD][wallIt+1] << MAP_UNIT_POWER;
    w2X = worlds[CUR_WORLD][wallIt+2] << MAP_UNIT_POWER;
    w2Y = worlds[CUR_WORLD][wallIt+3] << MAP_UNIT_POWER;
    rayDist = getRayToLineSegmentIntersection(posX, posY, globalRayDirX,
        globalRayDirY, w1X, w1Y, w2X, w2Y, &intWall);
#ifdef DEBUG
    printf("\r\n(%D,%D)-(%D,%D): %D", w1X, w1Y, w2X, w2Y,rayDist);
#endif
    if (rayDist > 0) {
      found = 1;
      if (w1X == w2X) {
        angle = globalRayAngle;
        sides[CUR_WORLD][wallIt/2] = SIDE_VER;
      } else {
        angle = globalRayAngle;
        sides[CUR_WORLD][wallIt/2] = SIDE_HOR;        
      }
      wallDist[CUR_WORLD][wallIt/2] = correctDist(rayDist, angle, globalRayDirX, globalRayDirY);
      intersection[CUR_WORLD][wallIt/2] = intWall;

#ifdef DEBUG
      printf(",%D", wallDist[CUR_WORLD][wallIt/2]);
#endif
    } else {
      wallDist[CUR_WORLD][wallIt/2] = TOO_FAR;
    }
    wallIt = wallIt + 2;
  }
  if (found) {
    rayDist = TOO_FAR;
    for (; i < WORLD_0_SIZE/2 - 1; i++) {
      if (wallDist[CUR_WORLD][i] < rayDist) {
        rayDist = wallDist[CUR_WORLD][i];
        wallIdx = i;
      }
    }
    *totalDistance = rayDist;
    *side = sides[CUR_WORLD][wallIdx];
    *wallTexture = wallTextures[CUR_WORLD][wallIdx];
    *textureX = intersection[CUR_WORLD][wallIdx] % MAP_UNIT_SIZE;
#ifdef DEBUG
    printf("\r\n-td:%D", *totalDistance);
#endif    
    return 0;
  } else {  
#ifdef DEBUG
    printf("\r\n-td: nocast");
#endif      
    return 1;
  }
}

void getWallSlice(uint8_t correctDist, uint8_t* drawStart, uint8_t* drawEnd) {
  uint8_t lineHeight = MAP_UNIT_SIZE * 35 / correctDist;

  // calculate lowest and highest pixel to fill in current stripe
  *drawStart = (SCREEN_HEIGHT - lineHeight) >> 1;
  
  if (*drawStart >= SCREEN_HEIGHT) *drawStart = 0;
  *drawEnd = (lineHeight + SCREEN_HEIGHT) >> 1;
  if (*drawEnd >= SCREEN_HEIGHT) *drawEnd = SCREEN_HEIGHT - 1;
}

