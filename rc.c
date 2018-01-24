#include <c64.h>
#include <cbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <peekpoke.h>
#include "rc.h"


uint8_t worldMap[MAP_HEIGHT][MAP_WIDTH]=
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,1,1,1,0,2,2,2,0,2,2,2,0,2,2,2,2,2,2},
  {1,0,0,0,0,0,1,1,1,1,2,0,0,0,0,0,2,0,0,0,0,0,0,2},
  {1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
  {1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,0,2},
  {1,1,0,1,1,1,1,1,1,1,2,2,2,0,2,2,2,2,2,0,0,2,2,2},
  {1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,1},
  {1,1,1,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,1},
  {1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1},
  {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1},
  {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};


void initTextures() {  
  uint8_t x, y, color;
  // Brick
  for (y = 0; y < TEXTURE_SIZE; y++) {
    for (x = 0; x < TEXTURE_SIZE; x++) {
      color = 0;
      if (x >= 10 && x < 22 && y > 10 && y < 20) {
        color = 1;
      }
      textureBrick[x*TEXTURE_SIZE+y] = color;
    }
  }

  // Square
  for (y = 0; y < TEXTURE_SIZE; y++) {
    for (x = 0; x < TEXTURE_SIZE; x++) {
      color = 0;
      if ((x >= 10 && x < 13 || x >= 20 && x < 22) || 
        (y >= 10 && y < 13 || y >= 20 && y < 22)) {
        color = 1;
      }
      textureSquare[x*TEXTURE_SIZE+y] = color;
    }
  }
}


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

void initTextureScaleMap() {
  uint8_t i = 1;
  for(; i < SCREEN_HEIGHT; i++) {
    textureScaleMap[i] = (uint16_t)(TEXTURE_SIZE_FIXED_PART / i);
  }
}
  

// sidesMap[y][x] contains a 8-bit mask DDCCBBAA, where
// AA = 00 if northern side of wall is hidden,
// BB = 00 if eastern side of wall is hidden,
// CC = 00 if southern side of wall is hidden,
// DD = 00 if western side of wall is hidden.
// For visible sides, 11 is stored.
// Example: for middle wall here we store 00110011 = 0x33.
// 111
// 000
// 111
void compileMapSides() {
  uint8_t x, y, sidesMask;
  for (y = 0; y < MAP_HEIGHT; y++) {
    for (x = 0; x < MAP_WIDTH; x++) {    
      sidesMask = 0xFF;
      if (y == 0 || worldMap[y-1][x] > 0) {
        sidesMask &= 0XFC;
#ifdef DEBUG      
        if (y == 16 && x == 7) {
          printf("\n0xfc=%d", (int)sidesMask);
        }
#endif  
      }
      if (x == MAP_WIDTH - 1 || worldMap[y][x+1] > 0) {
        sidesMask &= 0xF3;
#ifdef DEBUG      
        if (y == 16 && x == 7) {
          printf("\n0xf3=%d", (int)sidesMask);
        }
#endif        
      }
      if (y == MAP_HEIGHT - 1 || worldMap[y+1][x] > 0) {
        sidesMask &= 0xCF;
#ifdef DEBUG      
        if (y == 16 && x == 7) {
          printf("\n0xcf=%d", (int)sidesMask);
        }
#endif
      }
      if (x == 0 || worldMap[y][x-1] > 0) {
        sidesMask &= 0x3F;
#ifdef DEBUG      
        if (y == 16 && x == 7) {
          printf("\n0x3f=%d", (int)sidesMask);
        }
#endif                
      }
      sidesMap[y][x] = sidesMask;
#ifdef DEBUG      
      if (y == 16 && x == 7) {
        printf("AAA %d", (int)sidesMap[y][x]);
      }
#endif      
    }
  }
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
   

/*
uint8_t isEastVisible(uint8_t sidesMask) {
  return (sidesMask & 0x0C) >> 2;
}

uint8_t isSouthVisible(uint8_t sidesMask) {
  return (sidesMask & 0x30) >> 4;
}

uint8_t isWestVisible(uint8_t sidesMask) {
  return (sidesMask & 0xC0) >> 6;
}
*/

uint8_t getColor(uint8_t mapValue, uint8_t side, uint16_t numTextureCols, uint8_t textureY) {
  uint8_t* texture;
  uint8_t* colors;
  uint8_t colorIndex;

  if (mapValue == 1) {
    texture = textureBrick;
    colors = textureBrickColors;
  } else if (mapValue == 2) {
    texture = textureSquare;
    colors = textureSquareColors;
  }  
  colorIndex = texture[numTextureCols+textureY];
  if (side == SIDE_HOR) {
    colorIndex += 2;
  }
  return colors[colorIndex];
}

void verLine(uint8_t x, uint8_t start, uint8_t end, uint8_t side, uint8_t textureX, uint8_t mapValue) {
  uint16_t offset;
  uint16_t colorMapOffset;
  uint16_t charOutAddr;
  uint8_t* colorOutAddr;
  uint16_t numTextureCols;

  uint16_t scale;
  uint8_t textureY = 0;
  uint16_t textureYFrac = 0;
  uint8_t y = 0;
  
  scale = textureScaleMap[end-start];
  numTextureCols = textureX * TEXTURE_SIZE;
#ifdef DEBUG        
  printf("\nvl: t=%u,u=%u,s=%u ", TEXTURE_SIZE_FIXED_PART, (end-start), scale);
#endif   
  for (; y < SCREEN_HEIGHT; y++) {
    offset = x + SCREEN_WIDTH * y;
    charOutAddr = backCharBufAddr + offset; 
    colorOutAddr = &(backColorBuf[offset]);
    colorMapOffset = 0xd800 + offset;   
    if (y >= start && y < end) {
      textureY = textureYFrac >> 10;
      POKE(charOutAddr, 160);
      POKE(colorOutAddr, getColor(mapValue, side, numTextureCols, textureY));
      //POKE(0xd800+offset, textureBrick[textureX][textureY]);
      textureYFrac += scale;
#ifdef DEBUG        
      printf("%u,%u,%u ", textureYFrac, textureX, textureY);
#endif      
    } else if (y >= end) {
        POKE(charOutAddr, 160);
        POKE(colorOutAddr, 9);  //POKE(colorMapOffset, 9);
    } else {
      POKE(charOutAddr, 32);
      POKE(colorOutAddr, 0);  //POKE(colorMapOffset, 0);      
    }
  }  
}

// Flushes back- and color-buffer to video RAM.
void flushBuffer() {
  if (backCharBufAddr == 0x400) {
    POKE(0xD018, 0x15);
    backCharBufAddr = 0x800;
  } else {
    POKE(0xD018, 0x25);
    backCharBufAddr = 0x400;
  }  
  memcpy((uint8_t *)0xd800, backColorBuf, 1000);  
}

uint8_t normalizeAngle(int16_t angle) {
  int16_t res = angle % 256;
  if (res < 0) {
    res += 256;
  }
  return (uint8_t)res;
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


int main (void) {
  uint8_t pixToBrad; 
  // All angles are in brad, 0..255
  
  // Index of vertical stripe, in screen coordinates (0..SCREEN_WIDTH).
  uint8_t x;
  // Global angle of ray direction.
  int8_t localAngle;
  uint8_t globalRayAngle;
  // Start global camera position.
  int16_t posX = 23 * 32 + 16 - 4, posY = 1 * 32 + 16;
  // Start global camera direction.
  uint8_t cameraAngle = 128;
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
  int16_t /*xa, ya,*/ ax, ay, bx, by;
  uint8_t axMap, ayMap, bxMap, byMap; // A and B in map coordinates.
  int16_t pa, pb;
  uint16_t totalDist, correctDist;  
  uint8_t sidesMaskA, sidesMaskB;
  uint16_t newPosX, newPosY;  // Used for collision detection.
  uint8_t mapValue;
  
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
      localAngle = x * pixToBrad - SCREEN_WIDTH / 2;
      globalRayAngle = normalizeAngle(cameraAngle - localAngle);
      globalRayDirX = COS[globalRayAngle]; 
      globalRayDirY = -SIN[globalRayAngle];  // Inverse as Y axis goes down.
      globalRayPosX = posX;
      globalRayPosY = posY;
#ifdef DEBUG      
        printf("\ntracing ray %d: a=%d,gx=%d,gy=%d", x, globalRayAngle,globalRayDirX,globalRayDirY);
#endif      
      //which box of the map we're in
      mapX = globalRayPosX >> 5;
      mapY = globalRayPosY >> 5;
   
      if (globalRayDirY <= 0) {
        ay = (globalRayPosY & 0xFFE0) - 1;          
      } else {
        ay = (globalRayPosY & 0xFFE0) + 32;
      }
      if (globalRayDirX <= 0) {
        bx = (globalRayPosX & 0xFFE0) - 1;
      } else {
        bx = (globalRayPosX & 0xFFE0) + 32;
      }
      ayMap = ay >> 5;
      bxMap = bx >> 5;      
      side = SIDE_UNDEF;
      totalDist = 0;

      // Start DDA.
      while (side == SIDE_UNDEF) {      
#ifdef DEBUG
        printf("\n-grpx=%d,grpy=%d", globalRayPosX, globalRayPosY);
#endif        
        // Not using TAN here to keep higher precision:
        // E.g. ((2<<5)*28/15)>>5=3; with TAN, ((2<<5)*1)>>5=2.
        ax = globalRayPosX + ((((globalRayPosY-ay) << 5) *
          globalRayDirX / -globalRayDirY) >> 5);        
        by = globalRayPosY + ((((globalRayPosX-bx) << 5) *
          -globalRayDirY / globalRayDirX) >> 5);
        axMap = ax >> 5;
        byMap = by >> 5;
        pa = distance(globalRayAngle, globalRayPosX, globalRayPosY, ax, ay);
        pb = distance(globalRayAngle, globalRayPosX, globalRayPosY, bx, by);
        // Handle perpendiculars where overflow happens.
        if (pa < 0) pa = TOO_FAR;
        if (pb < 0) pb = TOO_FAR;
        if (abs(pa-pb) <= 5) {   // With 4 errors still exist.
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
          textureX = ax % 32;
        } else {
          if (worldMap[byMap][bxMap] > 0) {  
            side = SIDE_VER;
            mapX = bxMap;
            mapY = byMap;
          }
          globalRayPosX = bx;
          globalRayPosY = by;
          totalDist += pb;
          textureX = by % 32;
        }
        
#ifdef DEBUG        
        printf("\n-td=%d", totalDist);
#endif        
        
        if (globalRayDirY <= 0) {
          ay = (globalRayPosY & 0xFFE0) - 1;
          ayMap = ay >> 5;
        } else {
          ay = (globalRayPosY & 0xFFE0) + 32;
          ayMap = ay >> 5;          
        }
        if (globalRayDirX <= 0) {
          bx = (globalRayPosX & 0xFFE0) - 1;
          bxMap = bx >> 5;
          //xa = -xa;
        } else {
          bx = (globalRayPosX & 0xFFE0) + 32;
          bxMap = bx >> 5;          
        }                
      }
#ifdef DEBUG
      printf("\n*mx=%d,my=%d,s=%d,td=%d", mapX, mapY, side, totalDist);
#endif      
      correctDist = (totalDist * COS[normalizeAngle(localAngle)]) >> 5;
      
      

      //Calculate height of line to draw on screen
      lineHeight = 32*35 / correctDist;

      //calculate lowest and highest pixel to fill in current stripe
      drawStart = (SCREEN_HEIGHT -lineHeight) >> 1;
      //if(drawStart < 0)drawStart = 0;
      if (drawStart >= SCREEN_HEIGHT) drawStart = 0;
      drawEnd = (lineHeight + SCREEN_HEIGHT) >> 1;
      if(drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

      //draw the pixels of the stripe as a vertical line     
//#ifndef DEBUG      
      mapValue = worldMap[mapY][mapX];
      verLine(x, drawStart, drawEnd, side, textureX, mapValue);
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
    flushBuffer();
    
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

