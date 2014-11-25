#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "blitter.h"
#include "2d.h"
#include "fx.h"
#include "random.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 5
#define PNUM 8

static BitmapT *screen[2];
static UWORD active = 0;
static CopInsT *bplptr[5];

static BitmapT *neon[2];
static BitmapT *background;
static PaletteT *palette;
static CopListT *cp;
static CopInsT *pal;

static Point2D p[PNUM];
static Point2D p_last[2][PNUM];

static void Load() {
  neon[0] = LoadILBM("data/greet_ada.ilbm");
  neon[1] = LoadILBM("data/greet_dcs.ilbm");
  background = LoadILBM("data/neons.ilbm");
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  cp = NewCopList(100);
  palette = NewPalette(32);

  {
    WORD i = 0, j;

    for (j = 0; j < 16; j++, i++)
      palette->colors[i] = background->palette->colors[i];
    for (j = 0; j < 8; j++, i++)
      palette->colors[i] = neon[0]->palette->colors[i];
    for (j = 0; j < 8; j++, i++)
      palette->colors[i] = neon[1]->palette->colors[i];

    DeletePalette(neon[0]->palette);
    DeletePalette(neon[1]->palette);
    DeletePalette(background->palette);
  }
}

static void UnLoad() {
  DeleteCopList(cp);
  DeleteBitmap(neon[0]);
  DeleteBitmap(neon[1]);
  DeleteBitmap(background);
  DeletePalette(palette);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static void RotatePalette() {
  WORD f = frameCount * 128;
  WORD i;

  for (i = 1; i < 16; i++) {
    ColorT c = palette->colors[(i + frameCount) & 15];
    WORD r = c.r + normfx(SIN(f) * c.r) / 4;
    WORD g = c.g + normfx(SIN(f) * c.g) / 4;
    WORD b = c.b + normfx(SIN(f) * c.b) / 4;

    if (r < 0) r = 0;
    if (r > 255) r = 255;
    if (g < 0) g = 0;
    if (g > 255) g = 255;
    if (b < 0) b = 0;
    if (b > 255) b = 255;

    CopInsSetRGB24(&pal[i], r, g, b);
  }
}

static void VBlankInterrupt() {
  if (custom->intreqr & INTF_VERTB)
    RotatePalette();
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[active], DEPTH);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  pal = CopLoadPal(cp, palette, 0);
  CopEnd(cp);
}

static void Init() {
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;

  ITER(i, 0, PNUM - 1, p[i].x = 0);
  ITER(i, 0, PNUM - 1, p[i].y = -128);

  ITER(i, 0, 3, BlitterCopySync(screen[0], i, 0, 0, background, i));
  ITER(i, 0, 3, BlitterCopySync(screen[1], i, 0, 0, background, i));

  BlitterClearSync(screen[0], 4);
  BlitterClearSync(screen[1], 4);

  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
  custom->intena = INTF_SETCLR | INTF_VERTB;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER;
  custom->intena = INTF_VERTB;
}

static void ClearCliparts() {
  WORD i;

  for (i = 0; i < PNUM; i++) {
    BitmapT *src = neon[i & 1];
    Area2D s = {0, 0, src->width, 8};
    Point2D d = {p_last[active][i].x, p_last[active][i].y + src->height - 4};

    if (ClipArea2D(&d, WIDTH, HEIGHT, &s)) {
      BlitterSetSync(screen[active], 4, d.x, d.y, s.w, s.h, 0x0000);
      ITER(i, 0, 3, BlitterCopyAreaSync(screen[active], i, d.x, d.y,
                                        background, i, d.x, d.y, s.w, s.h));
    }
  }
}

static void DrawCliparts() {
  WORD step = frameCount - lastFrameCount;
  WORD i;

  for (i = 0; i < PNUM; i++) {
    BitmapT *src = neon[i & 1];
    Area2D s = {0, 0, src->width, src->height};
    Point2D d = {p[i].x, p[i].y};

    if (ClipArea2D(&d, WIDTH, HEIGHT, &s)) {
      ITER(i, 0, 2, BlitterCopyAreaSync(screen[active], i, d.x, d.y,
                                        src, i, s.x, s.y, s.w, s.h));
      ITER(i, 3, 4, BlitterSetSync(screen[active], i, d.x, d.y, s.w, s.h, 0xffff));
    } else {
      if (p[i].y < 0) {
        p[i].x = 96 + (random() % 112);
        p[i].y = 256 + (random() & 255);
      }
    }

    p_last[active][i].x = p[i].x;
    p_last[active][i].y = p[i].y;

    p[i].y -= step;
  }
}

static void Render() {
  ClearCliparts();
  DrawCliparts();

  WaitVBlank();
  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, VBlankInterrupt };
