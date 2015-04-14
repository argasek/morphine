#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/circle.h"
#include "gfx/line.h"
#include "tools/frame.h"
#include "tools/loopevent.h"

#include "system/c2p.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Canvas", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Buffer", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
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
  PixBufClear(R_("Canvas"));
  PixBufClear(R_("Buffer"));
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
}

/*
 * Effect rendering functions.
 */
int effect = 0;
const int lastEffect = 2;

void RenderDiskWithFgColorSwap(PixBufT *canvas, int adjust, int diameter, const int x, const int y) {
  int radius, adjustedDiameter = diameter - adjust;
  if (adjustedDiameter > 1) {
    radius = adjustedDiameter >> 1;
    canvas->fgColor = (255 - canvas->fgColor);
    DrawDisk(canvas, x - radius, y - radius, adjustedDiameter);
  }
}

void RenderDiskWithDot(PixBufT *canvas, int diameter, const int x, const int y) {
  int adjust, radius = diameter >> 1;
  DrawDisk(canvas, x - radius, y - radius, diameter);
  adjust = (diameter / 10);
  RenderDiskWithFgColorSwap(canvas, adjust, diameter, x, y);
  adjust = (diameter / 6);
  RenderDiskWithFgColorSwap(canvas, adjust, diameter, x, y);
  adjust = (diameter / 2);
  RenderDiskWithFgColorSwap(canvas, adjust, diameter, x, y);
}

void DrawTripleLine(PixBufT *buffer, int y) {
  buffer->fgColor = 64;
  DrawLineUnsafe(buffer, 0, y, WIDTH - 1, y);
  buffer->fgColor = 32;
  DrawLineUnsafe(buffer, 0, y - 1, WIDTH - 1, y - 1);
  DrawLineUnsafe(buffer, 0, y + 1, WIDTH - 1, y + 1);
}

void RenderDisks(int frameNumber) {
  PixBufT *buffer = R_("Buffer");
  PixBufT *canvas = R_("Canvas");
  int diameter = 64, hSpace = 0, cx, cy, y;
  int i;

  PixBufClear(buffer);

  frameNumber = frameNumber << 2;
  frameNumber &= 255;

  hSpace = 0;

  if (effect == 0) {
    diameter = 64;

    cx = diameter;
    cy = HEIGHT >> 1;

    y = 64 + 9;
    DrawTripleLine(buffer, y);

    y = 64 + 128 - 8;
    DrawTripleLine(buffer, y);

    buffer->fgColor = (frameNumber < 128) ? frameNumber * 2 : (255 - frameNumber) * 2;

    diameter = 64;
    RenderDiskWithDot(buffer, diameter, cx + hSpace, cy);

    hSpace += diameter + (diameter >> 1);
    diameter = 100;
    RenderDiskWithDot(buffer, diameter, cx + hSpace, cy);

    diameter = 64;
    hSpace += diameter + (diameter >> 1);
    RenderDiskWithDot(buffer, diameter, cx + hSpace, cy);

    DrawCircleAntialiased(buffer, (WIDTH - 256) >> 1, 0, 256);

  }

  if (effect == 1) {

    for (i = 256; i > 0; i -= 16) {
      buffer->fgColor = (frameNumber+i < 128) ? (frameNumber+i) * 2 : (255 - frameNumber+i) * 2;

      diameter = i;
      DrawCircleAntialiased(buffer, (WIDTH - diameter) >> 1, (HEIGHT - diameter) >> 1, diameter);

    }

    diameter = 4;

    DrawCircleAntialiased(buffer, (WIDTH - diameter) >> 1, (HEIGHT - diameter) >> 1, diameter);

  }

  PixBufCopy(canvas, buffer);
  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
  PixBufSwapData(canvas, buffer);
}

/*
 * Main loop.
 */
void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event != LOOP_CONTINUE) {
      PixBufClear(R_("Canvas"));

      if (event == LOOP_NEXT)
        effect = (effect + 1) % lastEffect;
      if (event == LOOP_PREV) {
        effect--;
        if (effect < 0)
          effect += lastEffect;
      }
    }

    RenderDisks(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
