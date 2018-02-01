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
#include "math.h"
#include "world.h"


////////////////////////////////////////////////////////////////////////////////
// Definitions.
////////////////////////////////////////////////////////////////////////////////

uint8_t textureBrick[TEXTURE_SIZE*TEXTURE_SIZE];
uint8_t textureBrickColors[4] = {5, 12, 13, 15};
uint8_t textureSquare[TEXTURE_SIZE*TEXTURE_SIZE];
uint8_t textureSquareColors[4] = {2, 8, 10, 7};
uint8_t textureScaleMap[SCREEN_HEIGHT];
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

void printIntro() {
  printf("\n");
  printf("***************************************\n");
  printf("* C64 RAYCASTER (C) S.BORODAVKIN 2018 *\n");
  printf("***************************************\n\n"); 
}

int main (void) {
  // Index of vertical stripe, in screen coordinates (0..SCREEN_WIDTH).
  uint8_t x;
  // Local angle of ray direction, relative to cameraAngle.
  int8_t localAngle;
  // Global ray angle.
  uint8_t globalRayAngle;
  uint8_t pixToBrad = FOV / SCREEN_WIDTH; 
  // Start global camera position.
  uint8_t posX = 2 * MAP_UNIT_SIZE + MAP_UNIT_SIZE/2,
          posY = 2 * MAP_UNIT_SIZE + MAP_UNIT_SIZE/2;
  // Start global camera direction.
  uint8_t cameraAngle = 224;
  uint8_t halfScreenWidth = SCREEN_WIDTH / 2;

  // Whether we have hit the wall, and from which side (0 = hor, 1 = vert)
  uint8_t side;
  uint8_t drawStart, drawEnd, textureX;
  // Input char
  char c;  
  uint16_t correctDist;  
  uint8_t newPosX, newPosY;  // Used for collision detection.
  uint8_t wallTexture;
  clock_t fpsStart, fpsEnd;
  
  printIntro();
  printf("Init textures...\n");
  initTextures();
  printf("Init texture scale map...\n");
  initTextureScaleMap();
  printf("Press a key...");
  waitForKey();   
    
  for(;;) {
    fpsStart = clock();
    for (x = 0; x < SCREEN_WIDTH; x++) {
      localAngle = x * pixToBrad - halfScreenWidth;
      globalRayAngle = cameraAngle - localAngle;
      
      if (castRay(posX, posY, globalRayAngle, localAngle, &correctDist,
          &side, &wallTexture, &textureX) == 1) {
        //printf("nocast");
        //waitForKey();
        //return 1;
      }
      //correctDist = (correctDist * COS[localAngle]) >> 4;
      getWallSlice(correctDist, &drawStart, &drawEnd);
#ifdef DEBUG      
      waitForKey();
#endif      
      verLine(x, drawStart, drawEnd, side, textureX,
          textureScaleMap[drawEnd-drawStart], wallTexture, backCharBufAddr,
          backColorBuf);
#ifdef DEBUG
      waitForKey();
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

