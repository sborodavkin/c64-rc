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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <peekpoke.h>
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
int8_t COS[NUM_ANGLES] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 2, 2, 2, 1, 1, 0, 0, 0, -1, -1, -2, -2, -2, -3, -3, -4, -4, -4, -5, -5, -5, -6, -6, -6, -7, -7, -8, -8, -8, -9, -9, -9, -10, -10, -10, -10, -11, -11, -11, -12, -12, -12, -12, -13, -13, -13, -13, -14, -14, -14, -14, -14, -14, -15, -15, -15, -15, -15, -15, -15, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -15, -15, -15, -15, -15, -15, -15, -14, -14, -14, -14, -14, -14, -13, -13, -13, -13, -12, -12, -12, -12, -11, -11, -11, -10, -10, -10, -10, -9, -9, -9, -8, -8, -8, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -2, -2, -2, -1, -1, 0, 0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16};
int8_t SIN[NUM_ANGLES] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 2, 2, 2, 1, 1, 0, 0, 0, -1, -1, -2, -2, -2, -3, -3, -4, -4, -4, -5, -5, -5, -6, -6, -6, -7, -7, -8, -8, -8, -9, -9, -9, -10, -10, -10, -10, -11, -11, -11, -12, -12, -12, -12, -13, -13, -13, -13, -14, -14, -14, -14, -14, -14, -15, -15, -15, -15, -15, -15, -15, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -15, -15, -15, -15, -15, -15, -15, -14, -14, -14, -14, -14, -14, -13, -13, -13, -13, -12, -12, -12, -12, -11, -11, -11, -10, -10, -10, -10, -9, -9, -9, -8, -8, -8, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -2, -2, -2, -1, -1, 0};


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
   


// Normalizes angle to 0..255 brad.
// It works because a % b = a & (b-1) and casting to uint8_t
// is same as adding 256 if a number was negative.
// The original implementation was:
/*uint8_t normalizeAngle(int16_t angle) {
  int16_t res = angle % 256;
  if (res < 0) {
    res += 256;
  }
  return (uint8_t)res;
} */
#define normalizeAngle(angle) ((uint8_t)(angle & 0xFF))

// Helper to calculate a distance between two points on a ray with given angle.
uint16_t distance(uint8_t angle, uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by) {
  uint16_t distX = abs(ax-bx), distY = abs(ay-by);
  if (distX > distY) {
    // X is more distinct, use cos.v
    return (distX << MAP_UNIT_POWER) / abs(COS[angle]);
  } else {
    // Y is more distinct, use sin.
    return (distY << MAP_UNIT_POWER) / abs(SIN[angle]);
  }
}

void printPosInfo(uint8_t globalRayPosX, uint8_t globalRayPosY, uint8_t cameraAngle) {
  uint16_t charBufAddr;
  char res[32];
  uint8_t i;
  snprintf(res, sizeof(res), "(%u,%u),%u", globalRayPosX, globalRayPosY, cameraAngle);
  if (backCharBufAddr == CHAR_BUF_0) {
    charBufAddr = CHAR_BUF_1;
  } else {
    charBufAddr = CHAR_BUF_0;
  }
  for (i=0; i < strlen(res); i++) {
    *(uint8_t*)(charBufAddr + i) = res[i];
    *(uint8_t*)(0xd800 + i) = 0;
    
  }
}

#define DIFF(a, b) ((a > b) ? a - b : b - a)


int main (void) {
  uint8_t pixToBrad; 
  // All angles are in brad, 0..255
  
  // Index of vertical stripe, in screen coordinates (0..SCREEN_WIDTH).
  uint8_t x;
  // Global angle of ray direction.
  int8_t localAngle;
  uint8_t globalRayAngle;
  // Start global camera position.
  uint8_t posX = 60, /*14 * MAP_UNIT_SIZE + MAP_UNIT_SIZE/2,*/
          posY = 19; //1 * MAP_UNIT_SIZE + MAP_UNIT_SIZE/2;
  // Start global camera direction.
  uint8_t cameraAngle = 239;
  uint8_t halfScreenWidth = SCREEN_WIDTH / 2;
  // Global position of current ray (=camera position).
  int16_t globalRayPosX, globalRayPosY;
  // Global direction of current ray
  int8_t globalRayDirX, globalRayDirY;
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
  uint8_t pa, pb;  // TODO: artifacts
  uint16_t totalDist, correctDist;  
  uint8_t sidesMaskA, sidesMaskB;
  uint16_t newPosX, newPosY;  // Used for collision detection.
  uint8_t mapValue;
  uint8_t axbyCalcHelper;
  
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
    for (x = 0; x < SCREEN_WIDTH; x++) {
      localAngle = x * pixToBrad - halfScreenWidth;
      globalRayAngle = normalizeAngle(cameraAngle - localAngle);
      globalRayDirX = COS[globalRayAngle]; 
      globalRayDirY = -SIN[globalRayAngle];  // Inverse as Y axis goes down.
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
        //printf("\nololo grpxandfff0=%d,plus=%d\n", (globalRayPosY & 0xFFF0), (globalRayPosY & 0xFFF0) + MAP_UNIT_SIZE);
        ay = (globalRayPosY & 0xF0) + MAP_UNIT_SIZE;
      }
      if (globalRayDirX <= 0) {
        bx = (globalRayPosX & 0xF0) - 1;
      } else {
        bx = (globalRayPosX & 0xF0) + MAP_UNIT_SIZE;
      }
      ayMap = ay >> MAP_UNIT_POWER;
      bxMap = bx >> MAP_UNIT_POWER;      
      side = SIDE_UNDEF;
      totalDist = 0;

      // Start DDA.
      while (side == SIDE_UNDEF) {
        pb = 0; pa = 0;      
#ifdef DEBUG
        printf("\n-grpx=%d,grpy=%d", globalRayPosX, globalRayPosY);
#endif        
        if (abs(globalRayDirX) == 16 && abs(globalRayDirY) == 1) {
          // (Nearly) horizontal ray, won't cross any SIDE_HOR.
          ax = TOO_FAR;
          pa = TOO_FAR;
        } else {          
          ax = DIFF(globalRayPosY, ay) * abs(globalRayDirX) / abs(-globalRayDirY);
          if (globalRayDirX > 0) {
            ax = globalRayPosX + ax;
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
        if (abs(globalRayDirY) == 16 && abs(globalRayDirX) == 1) {
          // (Nearly) vertical ray, won't cross any SIDE_VER.
          by = TOO_FAR;
          pb = TOO_FAR;
        } else {          
          by = DIFF(globalRayPosX, bx) * abs(-globalRayDirY) / abs(globalRayDirX);
          if (globalRayDirY > 0) {
            by = globalRayPosY + by;
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
        if (!pa) pa = distance(globalRayAngle, globalRayPosX, globalRayPosY, ax, ay);
        if (!pb) pb = distance(globalRayAngle, globalRayPosX, globalRayPosY, bx, by);
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
            side = SIDE_HOR;
            mapX = axMap;
            mapY = ayMap;            
          }
          globalRayPosX = ax;
          globalRayPosY = ay;
          totalDist += pa;
          textureX = ax % TEXTURE_SIZE;
        } else {
          if (worldMap[byMap][bxMap] > 0) {  
            side = SIDE_VER;
            mapX = bxMap;
            mapY = byMap;
          }
          globalRayPosX = bx;
          globalRayPosY = by;
          totalDist += pb;
          textureX = by % TEXTURE_SIZE;
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
      correctDist = (totalDist * COS[normalizeAngle(localAngle)]) >> MAP_UNIT_POWER;
      
      

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
      mapValue = worldMap[mapY][mapX];
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
    printPosInfo(posX, posY, cameraAngle);
    
    cbm_k_chkin (0);
    c = cbm_k_getin();
    if (c) {
      if (c == 'd') {    
        cameraAngle = normalizeAngle((int16_t)(cameraAngle - 4));
      } else if (c == 'a') {
        cameraAngle = normalizeAngle((int16_t)(cameraAngle + 4));
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

