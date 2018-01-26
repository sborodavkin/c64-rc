#include <stdint.h>
#include "rc.h"


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
  if (side == 0) {
    colorIndex += 2;
  }
  return colors[colorIndex];
}

void verLine(uint8_t x, uint8_t start, uint8_t end, uint8_t side, uint8_t textureX, uint16_t scale, uint8_t mapValue, uint16_t backCharBufAddr, uint8_t* backColorBufAddr) {
  uint16_t offset;
  uint16_t colorMapOffset;
  uint8_t* charOutAddr;
  uint8_t* colorOutAddr;
  uint16_t numTextureCols = textureX << 5;  // * 32
  uint8_t textureY = 0;
  uint16_t textureYFrac = 0;
  uint8_t y = 0;
  
  while (y < 25) {
    offset = x + (y<<5) + (y<<3);  // faster than 40*y
    charOutAddr = (uint8_t*)(backCharBufAddr + offset);
    colorOutAddr = (uint8_t*)(backColorBufAddr + offset);
    colorMapOffset = 0xd800 + offset;   
    if (y >= start && y < end) {
      textureY = textureYFrac >> 10;
      *(uint8_t*)(charOutAddr) = (uint8_t)160;
      *(uint8_t*)(colorOutAddr) = (uint8_t)(getColor(mapValue, side, numTextureCols, textureY));
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
