#include <stdlib.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/pixbuf.h"
#include "gfx/palette.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

uint8_t FireMainTable[256 * 4];
uint8_t FireBorderTable[256 * 3];

void InitFireTables();
void CalculateFire(uint8_t *fire asm("a0"),
                   uint16_t width asm("d0"), uint16_t height asm("d1"));

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT + 2));
  ResAdd("Palette", NewPalette(256));
}

/*
 * Set up display function.
 */
bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

/*
 * Set up effect function.
 */
void SetupEffect() {
  PaletteT *palette = R_("Palette");
  RGB *colors = palette->colors;
  int i;

  /* create a suitable fire palette, this is crucial for a good effect */
  /* black to blue, blue to red, red to yellow, yellow to white*/
  for (i = 0; i < 32; i++) {
    /* black to blue, 32 values*/
    colors[i].b = i << 1;

    /* blue to red, 32 values*/
    colors[i + 32].r = i << 3;
    colors[i + 32].b =  64 - (i << 1);

    /*red to yellow, 32 values*/
    colors[i + 64].r = 255;
    colors[i + 64].g = i << 3;

    /* yellow to white, 162 */
    colors[i + 96].r = 255;
    colors[i + 96].g = 255;
    colors[i + 96].b = i << 2;
    colors[i + 128].r = 255;
    colors[i + 128].g = 255;
    colors[i + 128].b = 64 + (i << 2);
    colors[i + 160].r = 255;
    colors[i + 160].g = 255;
    colors[i + 160].b = 128 + (i << 2);
    colors[i + 192].r = 255;
    colors[i + 192].g = 255;
    colors[i + 192].b = 192 + i;
    colors[i + 224].r = 255;
    colors[i + 224].g = 255;
    colors[i + 224].b = 224 + i;
  } 

  LoadPalette(palette);
  StartProfiling();
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  StopProfiling();
}

/*
 * Effect rendering functions.
 */

static __regargs void IgniteBottom(uint8_t *fire, int16_t width) {
  /* draw random bottom line in fire array */

  do {
    int random = 1 + (int)(16.0f * (rand() / (RAND_MAX + 1.0f)));

    /* the lower the value, the intenser the fire, compensate a lower value with a higher decay value */
    *fire++ = (random > 9) ? 255 : 0;
  } while (--width > 0);
}

__regargs void RenderFire(PixBufT *canvas) {
  int16_t width = canvas->width;
  uint8_t *fire = &canvas->data[width * (canvas->height - 1)];

  InitFireTables();
  IgniteBottom(fire, width);
  CalculateFire(fire, width, 60);
}

void RenderEffect(int frameNumber) {
  PixBufT *canvas = R_("Canvas");

  PROFILE(Fire)
    RenderFire(canvas);
  PROFILE(C2P)
    c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
