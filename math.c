/******************************************************************************/
/* Ray casting engine for Commodore 64.                                       */
/* (C) 2018, Sergey Borodavkin                                                */
/* sergei.borodavkin@gmail.com                                                */
/*                                                                            */
/* math.c                                                                     */
/* Math routines implementation.                                              */
/*                                                                            */
/******************************************************************************/

#include <stdlib.h>
#include "math.h"
#include "raycaster.h"


#define isNorthVisible(sidesMask)    \
  (__AX__ = (sidesMask),             \
   asm("and #$03"),                  \
   __AX__)

#define isEastVisible(sidesMask)     \
  (__AX__ = (sidesMask),             \
   asm("and #$0C"),                  \
   asm("lsr"),                       \
   asm("lsr"),                       \
   __AX__)
   
#define isSouthVisible(sidesMask)    \
  (__AX__ = (sidesMask),             \
   asm("and #$30"),                  \
   asm("lsr"),                       \
   asm("lsr"),                       \
   asm("lsr"),                       \
   asm("lsr"),                       \
   __AX__)
   
#define isWestVisible(sidesMask)     \
  (__AX__ = (sidesMask),             \
   asm("and #$C0"),                  \
   asm("lsr"),                       \
   asm("lsr"),                       \
   asm("lsr"),                       \
   asm("lsr"),                       \
   asm("lsr"),                       \
   asm("lsr"),                       \
   __AX__)      

void castRay(uint8_t posX, uint8_t posY, uint8_t globalRayAngle, int8_t localAngle, uint8_t* totalDistance, uint8_t* side, uint8_t* mapValue, uint8_t* textureX) {
  // Which box of the map we're in, in coordinates of worldMap.
  uint8_t mapX, mapY;  
  // A is intersection with next horizontal line, B - with vertical line.
  uint8_t ax, ay, bx, by;
  uint8_t axMap, ayMap, bxMap, byMap; // A and B in map coordinates.
  // Distance to A and B, correspondingly.
  uint8_t pa, pb;
  // Distance to a wall prior to perspective correction.
  uint8_t totalDist;
  uint8_t sidesMaskA, sidesMaskB; 

  // Global position of current ray (=camera position).
  uint8_t globalRayPosX = posX,
          globalRayPosY = posY;          
  // Global direction of current ray
  int8_t globalRayDirX = COS[globalRayAngle],
         globalRayDirY = -SIN[globalRayAngle];  // Inverse as Y axis goes down.;
  // Helpers to keep absolute COS/SIN values.
  uint8_t absCos = abs(globalRayDirX),
          absSin = abs(globalRayDirY);

#ifdef DEBUG      
    printf("\ntracing ray: a=%d,gx=%d,gy=%d", globalRayAngle,globalRayDirX,globalRayDirY);
#endif      
  //which box of the map we're in
  mapX = globalRayPosX >> MAP_UNIT_POWER;
  mapY = globalRayPosY >> MAP_UNIT_POWER;

  getAyBx(globalRayPosX, globalRayPosY, globalRayDirX, globalRayDirY, &ay, &bx);

  ayMap = ay >> MAP_UNIT_POWER;
  bxMap = bx >> MAP_UNIT_POWER;      
  *side = SIDE_UNDEF;
  totalDist = 0;

  // Start DDA.
  while (*side == SIDE_UNDEF) {
    pb = 0; pa = 0;      
#ifdef DEBUG
    printf("\n-grpx=%d,grpy=%d", globalRayPosX, globalRayPosY);
#endif        
    if (absCos == 16 && absSin == 1) {
      // (Nearly) horizontal ray, won't cross any SIDE_HOR.
      ax = TOO_FAR;
      pa = TOO_FAR;
    } else {          
      ax = DIFF(globalRayPosY, ay) * absCos / absSin;
      if (globalRayDirX > 0) {
        if (globalRayPosX + ax > UINT8_MAX) {  // Uint8_t overflow.
          ax = TOO_FAR;
          pa = TOO_FAR;
        } else {          
          ax = globalRayPosX + ax;
        }
      } else {
        if (ax > globalRayPosX) {
          ax = TOO_FAR;
          pa = TOO_FAR;
        } else {
          ax = globalRayPosX - ax;
        }
      }
      axMap = ax >> MAP_UNIT_POWER;
    }
    if (absSin == 16 && absCos == 1) {
      // (Nearly) vertical ray, won't cross any SIDE_VER.
      by = TOO_FAR;
      pb = TOO_FAR;
    } else {          
      by = DIFF(globalRayPosX, bx) * absSin / absCos;
      if (globalRayDirY > 0) {
        if (globalRayPosY + by > UINT8_MAX) {  // Uint8_t overflow.
          by = TOO_FAR;
          pb = TOO_FAR;
        } else {
          by = globalRayPosY + by;
        }
      } else {
        if (by > globalRayPosY) {
          by = TOO_FAR;
          pb = TOO_FAR;
        } else {
          by = globalRayPosY - by;
        }
      }
      byMap = by >> MAP_UNIT_POWER;       
    }        
    if (!pa) pa = distance(absCos, absSin, globalRayPosX, globalRayPosY, ax, ay);
    if (!pb) pb = distance(absCos, absSin, globalRayPosX, globalRayPosY, bx, by);
    // Handle perpendiculars where overflow happens.
    if (abs(pa-pb) <= 5) {
      // Precision problem. Check actual sides visibility.
      sidesMaskA = sidesMap[ayMap][axMap];
      sidesMaskB = sidesMap[byMap][bxMap];
#ifdef DEBUG
      printf("\n-sma:%d,smb:%d", sidesMaskA, sidesMaskB);
#endif          
      if (globalRayDirY < 0 && !isSouthVisible(sidesMaskA)) {
        pa = TOO_FAR;
      } else if (globalRayDirY > 0 && !isNorthVisible(sidesMaskA)) {
        pa = TOO_FAR;
      } else {  // if (pa < TOO_FAR)
        if (globalRayDirX < 0 && !isEastVisible(sidesMaskB)) {
          pb = TOO_FAR;
        } else if (globalRayDirX > 0 && !isWestVisible(sidesMaskB)) {
          pb = TOO_FAR;
        }
      }
    }        
#ifdef DEBUG
      printf("\n-ax:%d,ay:%d,bx:%d,by:%d", ax, ay, bx, by);
      printf("\n-pa:%d,pb:%d", pa, pb);
#endif
    if (pa < pb) {
      if (worldMap[ayMap][axMap] > 0) {  
        *side = SIDE_HOR;
        mapX = axMap;
        mapY = ayMap;            
      }
      globalRayPosX = ax;
      globalRayPosY = ay;
      totalDist += pa;
      *textureX = ax % TEXTURE_SIZE;
    } else {
      if (worldMap[byMap][bxMap] > 0) {  
        *side = SIDE_VER;
        mapX = bxMap;
        mapY = byMap;
      }
      globalRayPosX = bx;
      globalRayPosY = by;
      totalDist += pb;
      *textureX = by % TEXTURE_SIZE;
    }
    
#ifdef DEBUG        
    printf("\n-td=%d", totalDist);
#endif        
    
    getAyBx(globalRayPosX, globalRayPosY, globalRayDirX, globalRayDirY, &ay, &bx);
    ayMap = ay >> MAP_UNIT_POWER;          
    bxMap = bx >> MAP_UNIT_POWER;
                    
  }
#ifdef DEBUG
  printf("\n*mx=%d,my=%d,s=%d,td=%d", mapX, mapY, *side, totalDist);
#endif      
  *totalDistance = (totalDist * COS[localAngle]) >> MAP_UNIT_POWER;
  *mapValue = worldMap[mapY][mapX];
}

void getWallSlice(uint8_t correctDist, uint8_t* drawStart, uint8_t* drawEnd) {
  uint8_t lineHeight = MAP_UNIT_SIZE*35 / correctDist;

  // calculate lowest and highest pixel to fill in current stripe
  *drawStart = (SCREEN_HEIGHT -lineHeight) >> 1;
  
  if (*drawStart >= SCREEN_HEIGHT) *drawStart = 0;
  *drawEnd = (lineHeight + SCREEN_HEIGHT) >> 1;
  if (*drawEnd >= SCREEN_HEIGHT) *drawEnd = SCREEN_HEIGHT - 1;
}



uint8_t distance(uint8_t absCos, uint8_t absSin, uint8_t ax, uint8_t ay,
                 uint8_t bx, uint8_t by) {
  uint8_t distX = DIFF(ax,bx), distY = DIFF(ay,by);
  if (distX > distY) {
    // X is more distinct, use cos.
    return (distX << MAP_UNIT_POWER) / absCos;
  } else {
    // Y is more distinct, use sin.
    return (distY << MAP_UNIT_POWER) / absSin;
  }
}

void getAyBx(uint8_t globalRayPosX, uint8_t globalRayPosY, int8_t globalRayDirX,
             int8_t globalRayDirY, uint8_t* ay, uint8_t* bx) {
  if (globalRayDirY <= 0) {
    *ay = (globalRayPosY & 0xF0) - 1;
  } else {
    *ay = (globalRayPosY & 0xF0) + MAP_UNIT_SIZE;
  }
  if (globalRayDirX <= 0) {
    *bx = (globalRayPosX & 0xF0) - 1;
  } else {
    *bx = (globalRayPosX & 0xF0) + MAP_UNIT_SIZE;
  }
}
