/******************************************************************************/
/* Ray casting engine for Commodore 64.                                       */
/* (C) 2018, Sergey Borodavkin                                                */
/* sergei.borodavkin@gmail.com                                                */
/*                                                                            */
/* raycaster.h                                                                */
/* Rendering routines.                                                        */
/*                                                                            */
/******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
// Definitions.
////////////////////////////////////////////////////////////////////////////////

// Character buffers. Every i-th frame is rendered to CHAR_BUF_0,
// (i+1)-th frame is rendered to CHAR_BUF_1, (i+2)-th - to CHAR_BUF_0 again
// and so on.
#define CHAR_BUF_0 0x400;
#define CHAR_BUF_1 0x800;

// Back char buffer index - 0 or 1.
extern uint16_t backCharBufAddr;

/* Draws the vertical line of a scene.
 *
 * Args:
 *   x: 0..SCREEN_WIDTH-1 - index of a vertical line being drawn.
 *   start: zero-based offset from which the wall slice begins.
 *   end: zero-based offset at which the wall slice ends.
 *   side: either SIDE_HOR or SIDE_VER - which side of the wall the ray crossed.
 *   textureX: X-coordinate of texel.
 *   scale: texture scale factor for given wall slice height.
 *   mapValue: worldMap value at position being drawn.
 *   backCharBufAddr: character buffer address.
 *   backColorBufAddr: color RAM back buffer address.
 */ 
extern void verLine(uint8_t x, uint8_t start, uint8_t end, uint8_t side,
                    uint8_t textureX, uint16_t scale, uint8_t mapValue,
                    uint16_t backCharBufAddr, uint16_t backColorBufAddr);

/* Gets color of a specific Y-offset of a wall slice.
 * Args:
 *   mapValue: worldMap value at position being drawn.
 *   side: either SIDE_HOR or SIDE_VER - which side of the wall the ray crossed.
 *   numTexelColsRendered: number of texel columns rendered so far.
 *                         For example, if so far we have drawn:
 *                        
 *                         xxx
 *                         xxx
 *                         xxx
 *                         xx
 *
 *                         then numTexelsRendered is 2 (2 "columns" of texels).
 *   textureY: Y-coordinate of texel that we need to draw (in the example above
 *             it is 3).
 */               
extern uint8_t getColor(uint8_t mapValue, uint8_t side,
                        uint16_t numTexelColsRendered, uint8_t textureY);
                        
/* Makes back char buffer visible and copies the back color buffer to color RAM.
 */
extern void flushFrame();
