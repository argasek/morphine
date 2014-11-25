#include "startup.h"
#include "2d.h"
#include "blitter.h"
#include "coplist.h"
#include "fx.h"
#include "memory.h"
#include "ilbm.h"

#define WIDTH  320
#define HEIGHT 256
#define DEPTH  5

static ShapeT *shape;
static PaletteT *palette;
static BitmapT *screen;
static CopInsT *bplptr[5];
static CopListT *cp;
static WORD plane, planeC;

static void Load() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  shape = LoadShape("data/boxes.2d");
  palette = LoadPalette("data/shapes-pal.ilbm");
  cp = NewCopList(100);
}

static void UnLoad() {
  DeleteShape(shape);
  DeleteCopList(cp);
  DeleteBitmap(screen);
  DeletePalette(palette);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopMakePlayfield(cp, bplptr, screen, DEPTH);
  CopLoadPal(cp, palette, 0);
  CopEnd(cp);
}

static void Init() {
  plane = screen->depth - 1;
  planeC = 0;

  /* Set up clipping window. */
  ClipWin.minX = fx4i(0);
  ClipWin.maxX = fx4i(319);
  ClipWin.minY = fx4i(0);
  ClipWin.maxY = fx4i(255);

  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;
}

static Point2D tmpPoint[2][16];

static __regargs void DrawPolygon(Point2D *out, WORD n) {
  WORD *pos = (WORD *)out;
  WORD x1, y1, x2, y2;

  x1 = *pos++ >> 4;
  y1 = *pos++ >> 4;
  n--;

  while (--n >= 0) {
    x2 = *pos++ >> 4;
    y2 = *pos++ >> 4;
    BlitterLineSync(x1, y1, x2, y2);
    x1 = x2; y1 = y2;
  }
}

static __regargs void DrawShape(ShapeT *shape) {
  Point2D *point = shape->viewPoint;
  PolygonT *polygon = shape->polygon;
  UBYTE *flags = shape->viewPointFlags;
  UWORD *vertex = shape->polygonVertex;
  UWORD polygons = shape->polygons;

  while (polygons--) {
    WORD i, n = polygon->vertices;
    UBYTE clipFlags = 0;
    UBYTE outside = 0xff;
    Point2D *in = tmpPoint[0];
    Point2D *out = tmpPoint[1];
    UWORD *vxptr = &vertex[polygon->index];

    for (i = 0; i < n; i++) {
      UWORD k = *vxptr++;
      clipFlags |= flags[k];
      outside &= flags[k];
      in[i] = point[k];
    }

    if (!outside) {
      n = ClipPolygon2D(in, &out, n, clipFlags);
      DrawPolygon(out, n);
    }

    polygon++;
  }
}

static void Render() {
  // LONG lines = ReadLineCounter();
  WORD i, a = frameCount * 64;
  Matrix2D t;

  BlitterClearSync(screen, plane);
  LoadIdentity2D(&t);
  Rotate2D(&t, frameCount * 8);
  Scale2D(&t, fx12f(1.0) + SIN(a) / 2, fx12f(1.0) + COS(a) / 2);
  Translate2D(&t, fx4i(screen->width / 2), fx4i(screen->height / 2));
  Transform2D(&t, shape->viewPoint, shape->origPoint, shape->points);
  PointsInsideBox(shape->viewPoint, shape->viewPointFlags, shape->points);
  WaitBlitter();
  BlitterLineSetup(screen, plane, LINE_EOR, LINE_ONEDOT);
  DrawShape(shape);
  BlitterFillSync(screen, plane);
  WaitBlitter();
  // Log("shape: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();

  for (i = 0; i < screen->depth; i++) {
    WORD j = (plane + i) % DEPTH;
    CopInsSet32(bplptr[i], screen->planes[j]);
  }

  if (planeC & 1)
    plane = (plane + 1) % DEPTH;

  planeC ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, NULL, Render };
