#include <graphics/gfxbase.h>
#include <proto/graphics.h>

#include "sprite.h"
#include "memory.h"

__regargs SpriteT *NewSprite(UWORD height, BOOL attached) {
  SpriteT *sprite = MemAlloc(sizeof(SpriteT), MEMF_PUBLIC|MEMF_CLEAR);

  sprite->height = height;
  sprite->data = MemAlloc((height ? (height + 2) : 1) * 4, MEMF_CHIP|MEMF_CLEAR);

  if (attached)
    sprite->attached = NewSprite(height, FALSE);

  return sprite;
}

__regargs SpriteT *NewSpriteFromBitmap(UWORD height, BitmapT *bitmap,
                                       UWORD xstart, UWORD ystart)
{
  SpriteT *sprite = NewSprite(height, bitmap->depth == 4);
  WORD yend = ystart + sprite->height;
  LONG start = ystart * bitmap->bytesPerRow / 2 + xstart / 16;
  LONG stride = bitmap->bytesPerRow / 2;

  if (bitmap->depth == 2) {
    UWORD *data = &sprite->data[2];
    UWORD *bpl0 = (UWORD *)bitmap->planes[0] + start;
    UWORD *bpl1 = (UWORD *)bitmap->planes[1] + start;

    for (; ystart < yend; ystart++) {
      *data++ = *bpl0;
      *data++ = *bpl1;

      bpl0 += stride;
      bpl1 += stride;
    }
  } else {
    UWORD *data0 = &sprite->data[2];
    UWORD *data1 = &sprite->attached->data[2];
    UWORD *bpl0 = (UWORD *)bitmap->planes[0] + start;
    UWORD *bpl1 = (UWORD *)bitmap->planes[1] + start;
    UWORD *bpl2 = (UWORD *)bitmap->planes[2] + start;
    UWORD *bpl3 = (UWORD *)bitmap->planes[3] + start;

    for (; ystart < yend; ystart++) {
      *data0++ = *bpl0;
      *data0++ = *bpl1;
      *data1++ = *bpl2;
      *data1++ = *bpl3;

      bpl0 += stride;
      bpl1 += stride;
      bpl2 += stride;
      bpl3 += stride;
    }
  }

  return sprite;
}

__regargs SpriteT *CloneSystemPointer() {
  struct SimpleSprite *sprite = GfxBase->SimpleSprites[0];
  UWORD height = sprite->height;
  SpriteT *pointer = NewSprite(height, FALSE);

  memcpy(pointer->data + 2, sprite->posctldata + 2, height * sizeof(LONG));

  return pointer;
}

__regargs void DeleteSprite(SpriteT *sprite) {
  if (sprite->attached)
    DeleteSprite(sprite->attached);

  MemFree(sprite->data, (sprite->height + 2) * 4);
  MemFree(sprite, sizeof(SpriteT));
}

__regargs void UpdateSprite(SpriteT *sprite) {
  UWORD hstart = sprite->x;
  UWORD vstart = sprite->y;
  UWORD vstop = vstart + sprite->height + 1;

  /*
   * SPRxPOS:
   *  Bits 15-8 contain the low 8 bits of VSTART
   *  Bits 7-0 contain the high 8 bits of HSTART
   *
   * SPRxCTL:
   *  Bits 15-8       The low eight bits of VSTOP
   *  Bit 7           (Used in attachment)
   *  Bits 6-3        Unused (make zero)
   *  Bit 2           The VSTART high bit
   *  Bit 1           The VSTOP high bit
   *  Bit 0           The HSTART low bit
   */

  UWORD sprpos = (vstart << 8) | ((hstart >> 1) & 255);
  UWORD sprctl = (vstop << 8) | ((vstart >> 6) & 4) | ((vstop >> 7) & 2) | (hstart & 1);

  sprite->data[0] = sprpos;
  sprite->data[1] = sprctl;

  if (sprite->attached) {
    sprite->attached->data[0] = sprpos;
    sprite->attached->data[1] = sprctl | 0x80;
  }
}

__regargs void UpdateSpritePos(SpriteT *sprite, UWORD hstart, UWORD vstart) {
  sprite->x = hstart;
  sprite->y = vstart;

  if (sprite->attached) {
    sprite->attached->x = hstart;
    sprite->attached->y = vstart;
  }

  UpdateSprite(sprite);
}
