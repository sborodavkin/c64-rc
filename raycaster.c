/******************************************************************************/
/* Ray casting engine for Commodore 64.                                       */
/* (C) 2018, Sergey Borodavkin                                                */
/* sergei.borodavkin@gmail.com                                                */
/*                                                                            */
/* raycaster.c                                                                */
/* Main module.                                                               */
/*                                                                            */
/******************************************************************************/

#include <c64.h>
#include <cbm.h>
#include <peekpoke.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "raycaster.h"
#include "render.h"


////////////////////////////////////////////////////////////////////////////////
// Definitions.
////////////////////////////////////////////////////////////////////////////////

uint8_t worldMap[MAP_HEIGHT][MAP_WIDTH] =
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,2,2,2,0,2,2,2,0,2,2,2,2,2,2},
  {1,1,2,0,0,0,0,0,2,0,0,0,0,0,0,2},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
  {1,0,2,0,0,0,0,0,2,0,0,0,0,0,0,2},
  {1,1,2,2,2,0,2,2,2,2,2,0,0,2,2,2},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

uint8_t textureBrick[TEXTURE_SIZE*TEXTURE_SIZE];
uint8_t textureBrickColors[4] = {5, 12, 13, 15};
uint8_t textureSquare[TEXTURE_SIZE*TEXTURE_SIZE];
uint8_t textureSquareColors[4] = {2, 8, 10, 7};
uint8_t textureScaleMap[SCREEN_HEIGHT];
uint8_t sidesMap[MAP_HEIGHT][MAP_WIDTH];
uint8_t backColorBuf[1000];
int8_t COS[NUM_ANGLES] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15,
    15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12,
    12, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 6, 6, 6, 5, 5, 5, 4,
    4, 4, 3, 3, 2, 2, 2, 1, 1, 0, 0, 0, -1, -1, -2, -2, -2, -3, -3, -4, -4, -4,
    -5, -5, -5, -6, -6, -6, -7, -7, -8, -8, -8, -9, -9, -9, -10, -10, -10, -10,
    -11, -11, -11, -12, -12, -12, -12, -13, -13, -13, -13, -14, -14, -14, -14,
    -14, -14, -15, -15, -15, -15, -15, -15, -15, -16, -16, -16, -16, -16, -16,
    -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16,
    -15, -15, -15, -15, -15, -15, -15, -14, -14, -14, -14, -14, -14, -13, -13,
    -13, -13, -12, -12, -12, -12, -11, -11, -11, -10, -10, -10, -10, -9, -9, -9,
    -8, -8, -8, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -2, -2, -2,
    -1, -1, 0, 0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 8, 8,
    8, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14,
    14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16,
    16, 16};

int8_t SIN[NUM_ANGLES] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6,
    6, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 12, 13,
    13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15,
    15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12,
    12, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 6, 6, 6, 5, 5, 5, 4,
    4, 4, 3, 3, 2, 2, 2, 1, 1, 0, 0, 0, -1, -1, -2, -2, -2, -3, -3, -4, -4, -4,
    -5, -5, -5, -6, -6, -6, -7, -7, -8, -8, -8, -9, -9, -9, -10, -10, -10, -10,
    -11, -11, -11, -12, -12, -12, -12, -13, -13, -13, -13, -14, -14, -14, -14,
    -14, -14, -15, -15, -15, -15, -15, -15, -15, -16, -16, -16, -16, -16, -16,
    -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16,
    -15, -15, -15, -15, -15, -15, -15, -14, -14, -14, -14, -14, -14, -13, -13,
    -13, -13, -12, -12, -12, -12, -11, -11, -11, -10, -10, -10, -10, -9, -9, -9,
    -8, -8, -8, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -2, -2, -2,
    -1, -1, 0};
    
char infoStrBuf[40];

////////////////////////////////////////////////////////////////////////////////
// Init routines.
////////////////////////////////////////////////////////////////////////////////

void initTextures() {  
  uint8_t x, y, color;
  // Brick
  for (y = 0; y < TEXTURE_SIZE; y++) {
    for (x = 0; x < TEXTURE_SIZE; x++) {
      color = 0;
      if (x >= 5 && x < 11 && y > 5 && y < 10) {
        color = 1;
      }
      textureBrick[x*TEXTURE_SIZE+y] = color;
    }
  }

  // Square
  for (y = 0; y < TEXTURE_SIZE; y++) {
    for (x = 0; x < TEXTURE_SIZE; x++) {
      color = 0;
      if ((x >= 5 && x < 7 || x >= 10 && x < 12) || 
        (y >= 5 && y < 7 || y >= 10 && y < 12)) {
        color = 1;
      }
      textureSquare[x*TEXTURE_SIZE+y] = color;
    }
  }
}

void initTextureScaleMap() {
  uint8_t i = 1;
  for(; i < SCREEN_HEIGHT; i++) {
    textureScaleMap[i] = (uint8_t)(TEXTURE_SIZE_FIXED_PART / i);
  }
}

void compileMapSides() {
  uint8_t x, y, sidesMask;
  for (y = 0; y < MAP_HEIGHT; y++) {
    for (x = 0; x < MAP_WIDTH; x++) {    
      sidesMask = 0xFF;
      if (y == 0 || worldMap[y-1][x] > 0) {
        sidesMask &= 0XFC;
      }
      if (x == MAP_WIDTH - 1 || worldMap[y][x+1] > 0) {
        sidesMask &= 0xF3;
      }
      if (y == MAP_HEIGHT - 1 || worldMap[y+1][x] > 0) {
        sidesMask &= 0xCF;
      }
      if (x == 0 || worldMap[y][x-1] > 0) {
        sidesMask &= 0x3F;
      }
      sidesMap[y][x] = sidesMask;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

uint8_t __fastcall__ (*cbm_k_getin)(void) = 0xFFE4;

uint8_t waitForKey() {
  uint8_t c;
  cbm_k_chkin (0);
  c = cbm_k_getin();
  while (!c) {
    cbm_k_chkin (0);
    c = cbm_k_getin();
  }
  return c;
}

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
   

// Helper to calculate a distance between two points on a ray with given angle.
uint8_t distance(uint8_t absCos, uint8_t absSin, uint8_t ax, uint8_t ay,
    uint8_t bx, uint8_t by) {
  uint8_t distX = abs(ax-bx), distY = abs(ay-by);
  if (distX > distY) {
    // X is more distinct, use cos.
    return (distX << MAP_UNIT_POWER) / absCos;
  } else {
    // Y is more distinct, use sin.
    return (distY << MAP_UNIT_POWER) / absSin;
  }
}

#define DIFF(a, b) ((a > b) ? a - b : b - a)

void printInfo(uint8_t posX, uint8_t posY, uint8_t angle, uint16_t clkFps) {
  uint16_t charBufAddr;
  uint8_t i;
  snprintf(infoStrBuf, sizeof(infoStrBuf), "(%d,%u),%u,%u", posX, posY, angle,
      clkFps);
  if (backCharBufAddr == CHAR_BUF_0) {
    charBufAddr = CHAR_BUF_1;
  } else {
    charBufAddr = CHAR_BUF_0;
  }
  for (i=0; i < strlen(infoStrBuf); i++) {
    *(uint8_t*)(charBufAddr + i) = infoStrBuf[i];
    *(uint8_t*)(0xd800 + i) = 0;    
  }
}

void castRay(uint8_t globalRayAngle, uint8_t* distance, uint8_t* side, uint8_t* mapValue, uint8_t* textureX) {
  // Start global camera position.
  uint8_t posX = 14 * MAP_UNIT_SIZE + MAP_UNIT_SIZE/2,
          posY = 1 * MAP_UNIT_SIZE + MAP_UNIT_SIZE/2;
  // Global position of current ray (=camera position).
  uint8_t globalRayPosX, globalRayPosY;          
  // Global direction of current ray
  int8_t globalRayDirX, globalRayDirY;
  uint8_t absCos, absSin;

  globalRayDirX = COS[globalRayAngle]; 
  globalRayDirY = -SIN[globalRayAngle];  // Inverse as Y axis goes down.
  absCos = abs(globalRayDirX);
  absSin = abs(globalRayDirY);
  globalRayPosX = posX;
  globalRayPosY = posY;
#ifdef DEBUG      
    printf("\ntracing ray %d: a=%d,gx=%d,gy=%d", x, globalRayAngle,globalRayDirX,globalRayDirY);
#endif      
  //which box of the map we're in
  mapX = globalRayPosX >> MAP_UNIT_POWER;
  mapY = globalRayPosY >> MAP_UNIT_POWER;

  if (globalRayDirY <= 0) {
    ay = (globalRayPosY & 0xF0) - 1;
  } else {
    ay = (globalRayPosY & 0xF0) + MAP_UNIT_SIZE;
  }
  if (globalRayDirX <= 0) {
    bx = (globalRayPosX & 0xF0) - 1;
  } else {
    bx = (globalRayPosX & 0xF0) + MAP_UNIT_SIZE;
  }
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
    
    if (globalRayDirY <= 0) {
      ay = (globalRayPosY & 0xF0) - 1;
      ayMap = ay >> MAP_UNIT_POWER;
    } else {
      ay = (globalRayPosY & 0xF0) + MAP_UNIT_SIZE;
      ayMap = ay >> MAP_UNIT_POWER;          
    }
    if (globalRayDirX <= 0) {
      bx = (globalRayPosX & 0xF0) - 1;
      bxMap = bx >> MAP_UNIT_POWER;
      //xa = -xa;
    } else {
      bx = (globalRayPosX & 0xF0) + MAP_UNIT_SIZE;
      bxMap = bx >> MAP_UNIT_POWER;          
    }                
  }
#ifdef DEBUG
  printf("\n*mx=%d,my=%d,s=%d,td=%d", mapX, mapY, side, totalDist);
#endif      
  *distance = (totalDist * COS[localAngle]) >> MAP_UNIT_POWER;
  *mapValue = worldMap[mapY][mapX];
}

int main (void) {
  uint8_t pixToBrad; 
  // All angles are in brad, 0..255
  
  // Index of vertical stripe, in screen coordinates (0..SCREEN_WIDTH).
  uint8_t x;
  // Global angle of ray direction.
  int8_t localAngle;
  uint8_t globalRayAngle;
  // Start global camera direction.
  uint8_t cameraAngle = 127;
  uint8_t halfScreenWidth = SCREEN_WIDTH / 2;

  // Which box of the map we're in, in coordinates of worldMap.
  uint8_t mapX, mapY;  
  // Whether we have hit the wall, and from which side (0 = hor, 1 = vert)
  uint8_t side;
  uint8_t lineHeight, drawStart, drawEnd, textureX;
  // Input char
  char c;  
  // A is intersection with next horizontal line, B - with vertical line.
  uint8_t /*xa, ya,*/ ax, ay, bx, by;
  uint8_t axMap, ayMap, bxMap, byMap; // A and B in map coordinates.
  uint8_t pa, pb;
  uint8_t totalDist, correctDist;  
  uint8_t sidesMaskA, sidesMaskB;
  uint8_t newPosX, newPosY;  // Used for collision detection.
  uint8_t mapValue;
  clock_t fpsStart, fpsEnd;
  
  printf("\n");
  printf("***************************************\n");
  printf("* C64 RAYCASTER (C) S.BORODAVKIN 2018 *\n");
  printf("***************************************\n\n");  
  printf("Compiling map sides...\n");
  compileMapSides();
  printf("Init textures...\n");
  initTextures();
  printf("Init texture scale map...\n");
  initTextureScaleMap();
  printf("Press a key...");
  waitForKey();   
  
  // Switch to ALL CAPS (cc65 switches to lowercase in startup code).
  // POKE(0xd018, 0x15);
  
  
  pixToBrad = FOV / SCREEN_WIDTH;
  
  for(;;) {
    fpsStart = clock();
    for (x = 0; x < SCREEN_WIDTH; x++) {
      localAngle = x * pixToBrad - halfScreenWidth;
      globalRayAngle = cameraAngle - localAngle;
      
      castRay(globalRayAngle, *correctDist, &side, &mapValue, &textureX);

      //Calculate height of line to draw on screen
      lineHeight = MAP_UNIT_SIZE*35 / correctDist;

      //calculate lowest and highest pixel to fill in current stripe
      drawStart = (SCREEN_HEIGHT -lineHeight) >> 1;
      //if(drawStart < 0)drawStart = 0;
      if (drawStart >= SCREEN_HEIGHT) drawStart = 0;
      drawEnd = (lineHeight + SCREEN_HEIGHT) >> 1;
      if(drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

      //draw the pixels of the stripe as a vertical line     
//#ifndef DEBUG      
      
      verLine(x, drawStart, drawEnd, side, textureX, textureScaleMap[drawEnd-drawStart], mapValue, backCharBufAddr, backColorBuf);
//#else
#ifdef DEBUG 
      printf("\n-cd=%d,lh=%d,ds=%d,de=%d,tx=%d", correctDist, lineHeight, drawStart, drawEnd, textureX);      
      cbm_k_chkin (0);
      c = cbm_k_getin();
      while (!c) {
        cbm_k_chkin (0);
        c = cbm_k_getin();
      }
#endif      
    }
    
#ifndef DEBUG
    flushFrame();
    fpsEnd = clock();
    printInfo(posX, posY, cameraAngle, fpsEnd-fpsStart);
    
    cbm_k_chkin (0);
    c = cbm_k_getin();
    if (c) {
      if (c == 'd') {    
        cameraAngle = cameraAngle - 4;
      } else if (c == 'a') {
        cameraAngle = cameraAngle + 4;
      } else if (c == 'w') {
        newPosX = posX + (COS[cameraAngle] >> 3);
        newPosY = posY + (-SIN[cameraAngle] >> 3);
        if (1) { //(worldMap[newPosY>>5][newPosX>>5] == 0) {
          posX = newPosX;
          posY = newPosY;
        }
      } else if (c == 's') {
        newPosX = posX - (COS[cameraAngle] >> 3);
        newPosY = posY - (-SIN[cameraAngle] >> 3);
        if (1) { //(worldMap[newPosY>>5][newPosX>>5] == 0) {
          posX = newPosX;
          posY = newPosY;
        }        
      }
    }
#endif    
  }
  return 0;
}

