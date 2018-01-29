/******************************************************************************/
/* Ray casting engine for Commodore 64.                                       */
/* (C) 2018, Sergey Borodavkin                                                */
/* sergei.borodavkin@gmail.com                                                */
/*                                                                            */
/* render.c                                                                   */
/* Render implementation.                                                     */
/*                                                                            */
/******************************************************************************/

#include <peekpoke.h>
#include <stdint.h>
#include <string.h>
#include "raycaster.h"
#include "render.h"

uint16_t backCharBufAddr = CHAR_BUF_1;

uint8_t getColor(uint8_t mapValue, uint8_t side, uint8_t numTexelColsRendered,
                 uint8_t textureY) {
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
  colorIndex = texture[numTexelColsRendered+textureY];
  if (side == SIDE_HOR) {
    colorIndex += 2;
  }
  return colors[colorIndex];
}

void verLine(uint8_t x, uint8_t start, uint8_t end, uint8_t side,
             uint8_t textureX, uint8_t scale, uint8_t mapValue,
             uint16_t backCharBufAddr, uint16_t backColorBufAddr) {
  uint16_t offset;
  uint8_t* charOutAddr;
  uint8_t* colorOutAddr;
  uint8_t numTexelColsRendered = textureX << MAP_UNIT_POWER;
  uint8_t textureY = 0;
  uint8_t textureYFrac = 0;
  uint8_t y = 0;
  while (y < SCREEN_HEIGHT) {
    offset = x + (y<<5) + (y<<3);  // Presumably faster than y*40.
    charOutAddr = (uint8_t*)(backCharBufAddr + offset);
    colorOutAddr = (uint8_t*)(backColorBufAddr + offset);
    if (y >= start && y < end) {
      textureY = textureYFrac >> 4;
      *(uint8_t*)(charOutAddr) = (uint8_t)160;
      *(uint8_t*)(colorOutAddr) = (uint8_t)(getColor(mapValue, side,
          numTexelColsRendered, textureY));
      textureYFrac += scale;
    } else if (y >= end) {
        *(unsigned char*)(charOutAddr) = (unsigned char)160;
        *(unsigned char*)(colorOutAddr) = (unsigned char)9;
    } else {
      *(unsigned char*)(charOutAddr) = (unsigned char)32;
      *(unsigned char*)(colorOutAddr) = (unsigned char)0;
    }
    y++;
  }
}

// Flushes back- and color-buffer to video RAM.
void flushFrame() {
  if (backCharBufAddr == CHAR_BUF_0) {
    // Set active buffer 0.
    POKE(0xD018, 0x15);
    // Set back buffer 1.
    backCharBufAddr = CHAR_BUF_1;
  } else {
    // Set active buffer 1.
    POKE(0xD018, 0x25);
    // Set back buffer 0.
    backCharBufAddr = CHAR_BUF_0;
  }  
  memcpy((uint8_t *)0xd800, backColorBuf, 1000);  
}

