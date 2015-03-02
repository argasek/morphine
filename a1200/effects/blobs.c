#include "std/debug.h"
#include "std/resource.h"

#include "gfx/blob.h"
#include "gfx/line.h"
#include "tools/frame.h"
#include "tools/loopevent.h"

#include "system/c2p.h"
#include "system/vblank.h"

const uint16_t WIDTH = 320;
const uint16_t HEIGHT = 256;
const uint8_t DEPTH = 8;

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
const int lastEffect = 1;

void DrawTripleLine(PixBufT *buffer, int y) {
  buffer->fgColor = 64;
  DrawLineUnsafe(buffer, 0, y, WIDTH - 1, y);
  buffer->fgColor = 32;
  DrawLineUnsafe(buffer, 0, y - 1, WIDTH - 1, y - 1);
  DrawLineUnsafe(buffer, 0, y + 1, WIDTH - 1, y + 1);
}

void RenderBlobs(int frameNumber) {
  PixBufT *buffer = R_("Buffer");
  PixBufT *canvas = R_("Canvas");
  uint16_t x, y;
  uint8_t activeColor;

  PixBufClear(buffer);

  frameNumber = frameNumber << 2;
  frameNumber &= 255;


  if (effect == 0) {
    activeColor = (uint8_t) ((frameNumber < 128) ? frameNumber * 2 : (255 - frameNumber) * 2);

    // Big blob
    x = (uint16_t) (WIDTH - 128) >> 1;
    y = (uint16_t) (HEIGHT - 128) >> 1;
    buffer->fgColor = activeColor;
    DrawBlob(buffer, x, y, 128);

    // Lines
    y = 64 + 9;
    DrawTripleLine(buffer, y);
    y = 64 + 128 - 8;
    DrawTripleLine(buffer, y);

    // Small blobs (left)
    x = 32;
    for (y = 16; y < HEIGHT-16; y += 24) {
      buffer->fgColor = (uint8_t) (y > 255 ? 255 : y);
      DrawBlob(buffer, 32, y, 16);
      DrawBlob(buffer, 320-32-17, y, 17);
    }

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

    RenderBlobs(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
