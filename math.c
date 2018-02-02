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

uint16_t getRayToLineSegmentIntersection(uint8_t posX, uint8_t posY, uint8_t angle,
    int8_t globalRayDirX, int8_t globalRayDirY,
    uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t* intersectionPointOnWall)
{
  uint8_t minWallCoord, maxWallCoord;  
  // tStar multitude is map dimension * 2^USTAR_SCALE, e.g. 256 * 128.
  // Experiments show that USTAR_SCALE=7 gives no overflow while at 8 there's
  // already some. TODO: if there are no large open scenes this might still be
  // fine.
  int16_t tStar, uStar, h;
  int16_t cox, coy;
  uint16_t iX, iY;

  if (x1 == x2) { 
    // Vertical line.
    if (abs(globalRayDirY) != 32) {
      uStar = USTAR_COS[angle];
      cox = (int16_t)x1 - (int16_t)posX;
      tStar = cox * uStar;
#ifdef DEBUG
      printf("\r\n-us=%D,cox=%D,ts=%D",uStar,cox,tStar);
#endif      
      if (tStar > 0) {
        h = posY + tStar * globalRayDirY;
        h = h / (1 << USTAR_SCALE);
        iY = (uint16_t)(posY + h);
#ifdef DEBUG
        printf(",h=%D,iy=%D",h,iY);
#endif              
        if (y1 < y2) {
          minWallCoord = y1;
          maxWallCoord = y2;
        } else {
          minWallCoord = y2;
          maxWallCoord = y1;
        }
        if POINT_IN_RANGE_AND_INCREMENT(iY, minWallCoord, maxWallCoord) {
          *intersectionPointOnWall = --iY;
          return distance(angle, posX, posY, x1, iY);
        }
      }
    }
    *intersectionPointOnWall = 0;
    return 0;
  } else {
    // Horizontal line.
    if (abs(globalRayDirX) != 32) {
      uStar = USTAR_SIN[angle];
      coy = (int16_t)y1 - (int16_t)posY;
      tStar = coy * uStar;
#ifdef DEBUG
      printf("\r\n-us=%D,coy=%D,ts=%D",uStar,coy,tStar);
#endif
      if (tStar > 0) {
        h = posX + tStar * globalRayDirX;
        h = h / (1 << USTAR_SCALE);
        iX = (uint16_t)(posX + h);
#ifdef DEBUG
        printf(",h=%D,ix=%D",h,iX);
#endif
        if (x1 < x2) {
          minWallCoord = x1;
          maxWallCoord = x2;
        } else {
          minWallCoord = x2;
          maxWallCoord = x1;
        }
        if POINT_IN_RANGE_AND_INCREMENT(iX, minWallCoord, maxWallCoord) {
          *intersectionPointOnWall = --iX;
          return distance(angle, posX, posY, iX, y1);
        }
      }
    }
    *intersectionPointOnWall = 0;
    return 0;  
  }    
}


// Helper to calculate a distance between two points on a ray with given angle.
uint16_t distance(uint8_t angle, uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by) {
  uint16_t distX = abs(ax-bx), distY = abs(ay-by);
  if (distX > distY) {
    // X is more distinct, use cos.v
    return (distX << 5) / abs(COS[angle]);
  } else {
    // Y is more distinct, use sin.
    return (distY << 5) / abs(SIN[angle]);
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
    rayDist = getRayToLineSegmentIntersection(posX, posY, globalRayAngle, globalRayDirX,
        globalRayDirY, w1X, w1Y, w2X, w2Y, &intWall);
#ifdef DEBUG
    printf("\r\n(%D,%D)-(%D,%D): %D", w1X, w1Y, w2X, w2Y,rayDist);
#endif
    if (rayDist > 0) {
      found = 1;
      if (w1X == w2X) {
        angle = 0x40 - globalRayAngle;
        sides[CUR_WORLD][wallIt/2] = SIDE_VER;
      } else {
        angle = globalRayAngle;
        sides[CUR_WORLD][wallIt/2] = SIDE_HOR;        
      }
      wallDist[CUR_WORLD][wallIt/2] = rayDist;
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

