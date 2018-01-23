#include <stdint.h>

#define MAP_WIDTH 24u
#define MAP_HEIGHT 24u
#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 25
#define TEXTURE_SIZE 32u
// 32768 is TEXTURE_SIZE << 10
#define TEXTURE_SIZE_FIXED_PART 32768u

//#define DEBUG 0

// FOV: 42 = 60 degrees
#define FOV 42

#define SIDE_HOR 0
#define SIDE_VER 1
#define SIDE_UNDEF 2

#define TOO_FAR 1024

// Texture is stored in vertical stripes, i.e. [x][y].
uint8_t textureBrick[TEXTURE_SIZE*TEXTURE_SIZE];

// TEXTURE_SIZE_FIXED_PART divided by all possible values of line height (1..SCREEN_HEIGHT)
uint16_t textureScaleMap[SCREEN_HEIGHT];

// Visible and hidden sides of walls.
uint8_t sidesMap[MAP_HEIGHT][MAP_WIDTH];

// Address of back char buffer.
uint16_t backCharBufAddr = 0x800;

// Back buffer for color map.
uint8_t backColorBuf[1000u];

#define NUM_ANGLES 256

// Cos table, 0-255 => -32..32
int8_t COS[NUM_ANGLES] = {32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 30,
    30, 30, 30, 29, 29, 29, 28, 28, 27, 27, 27, 26, 26, 25, 25, 24, 24,
    23, 23, 22, 21, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13,
    12, 12, 11, 10, 9, 9, 8, 7, 6, 5, 5, 4, 3, 2, 2, 1, 0, -1, -2, -2,
    -3, -4, -5, -5, -6, -7, -8, -9, -9, -10, -11, -12, -12, -13, -14,
    -14, -15, -16, -16, -17, -18, -18, -19, -20, -20, -21, -21, -22,
    -23, -23, -24, -24, -25, -25, -26, -26, -27, -27, -27, -28, -28,
    -29, -29, -29, -30, -30, -30, -30, -31, -31, -31, -31, -31, -32,
    -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32,
    -32, -31, -31, -31, -31, -31, -30, -30, -30, -30, -29, -29, -29,
    -28, -28, -27, -27, -27, -26, -26, -25, -25, -24, -24, -23, -23, 
    -22, -21, -21, -20, -20, -19, -18, -18, -17, -16, -16, -15, -14,
    -14, -13, -12, -12, -11, -10, -9, -9, -8, -7, -6, -5, -5, -4, -3,
    -2, -2, -1, 0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 8, 9, 9, 10, 11, 12, 12,
    13, 14, 14, 15, 16, 16, 17, 18, 18, 19, 20, 20, 21, 21, 22, 23, 23,
    24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 29, 29, 29, 30, 30, 30,
    30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32};
  
// Sin table, 0-255 => -32..32
int8_t SIN[NUM_ANGLES] = {0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 8, 9, 9, 10, 11, 12, 12,
    13, 14, 14, 15, 16, 16, 17, 18, 18, 19, 20, 20, 21, 21, 22, 23, 23,
    24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 29, 29, 29, 30, 30, 30,
    30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 31, 31, 31, 31, 31, 30, 30, 30, 30, 29, 29, 29, 28,
    28, 27, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 21, 21, 20, 20,
    19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 12, 12, 11, 10, 9, 9, 8, 7,
    6, 5, 5, 4, 3, 2, 2, 1, 0, -1, -2, -2, -3, -4, -5, -5, -6, -7, -8,
    -9, -9, -10, -11, -12, -12, -13, -14, -14, -15, -16, -16, -17, -18,
    -18, -19, -20, -20, -21, -21, -22, -23, -23, -24, -24, -25, -25,
    -26, -26, -27, -27, -27, -28, -28, -29, -29, -29, -30, -30, -30,
    -30, -31, -31, -31, -31, -31, -32, -32, -32, -32, -32, -32, -32,
    -32, -32, -32, -32, -32, -32, -32, -32, -31, -31, -31, -31, -31,
    -30, -30, -30, -30, -29, -29, -29, -28, -28, -27, -27, -27, -26,
    -26, -25, -25, -24, -24, -23, -23, -22, -21, -21, -20, -20, -19,
    -18, -18, -17, -16, -16, -15, -14, -14, -13, -12, -12, -11, -10,
    -9, -9, -8, -7, -6, -5, -5, -4, -3, -2, -2, -1};

// GETIN - get a character from default input device, missing in cbm.h
uint8_t __fastcall__ (*cbm_k_getin)(void) = 0xffe4;
uint8_t __fastcall__ (*cbm_k_clr)(void) = 0xe566;

void initTextures();
uint8_t waitForKey();
void compileMapSides();


// Draws the vertical line to backBuffer and colorBuffer.
void verLine(uint8_t x, uint8_t start, uint8_t end, uint8_t side, uint8_t textureX, uint8_t mapValue);

// Flushes back- and color-buffer to video RAM.
void flushBuffer();

// Normalizes angle to 0..255 brad.
uint8_t normalizeAngle(int angle);

// Helper to calculate a distance between two points on a ray with given angle.
uint16_t distance(uint8_t angle, uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by);
