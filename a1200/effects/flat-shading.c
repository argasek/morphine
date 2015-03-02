#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "engine/ms3d.h"
#include "engine/scene.h"
#include "gfx/blit.h"
#include "gfx/colorfunc.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "tools/frame.h"
#include "tools/gradient.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/vblank.h"

#include "uvmap/misc.h"
#include "uvmap/render.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAddPngImage("Texture", "TexturePal", "data/texture-shades.png");
  ResAddPngImage("ColorMap", NULL, "data/texture-shades-map.png");
  ResAdd("Map", NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256));
  ResAdd("Shades", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("Scene", NewScene());
  ResAdd("Mesh", NewMeshFromFile("data/shattered_ball.robj"));
  ResAdd("Orig", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));

  {
    MeshT *mesh = R_("Mesh");

    CenterMeshPosition(mesh);
    CalculateSurfaceNormals(mesh);
    NormalizeMeshSize(mesh);
  }

  SceneAddObject(R_("Scene"), NewSceneObject("Object", R_("Mesh")));

  RenderAllFaces = false;
  RenderMode = RENDER_FLAT_SHADING;
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
  UVMapT *uvmap = R_("Map");

  LoadPalette(R_("TexturePal"));

  UVMapGenerate2(uvmap);
  UVMapSetTexture(uvmap, R_("Texture"));

  ResAdd("Component", NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u));

  PixBufBlit(R_("Orig"), 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u), NULL);

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

static int EffectNum = 1;

void RenderEffect(int frameNumber) {
  PixBufT *canvas = R_("Canvas");
  UVMapT *uvmap = R_("Map");
  PixBufT *shades = R_("Shades");
  PixBufT *umap;

  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  SceneT *scene = R_("Scene");

  {
    MatrixStack3D *ms = GetObjectTranslation(scene, "Object");

    StackReset(ms);
    PushScaling3D(ms, 1.25f, 1.25f, 1.25f);
    PushRotation3D(ms, 0, (float)(-frameNumber * 2), frameNumber);
    PushTranslation3D(ms, 0.0f, 0.0f, -2.0f);
  }

  if (EffectNum == 0) {
    umap = NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u);

    PROFILE(PixBufClear)
      PixBufClear(shades);
    PROFILE(RenderScene)
      RenderScene(scene, shades);

    PixBufSetBlitMode(shades, BLIT_ADDITIVE);
    PixBufBlit(umap, 0, 0, R_("Orig"), NULL);
    PixBufBlit(umap, 0, 0, shades, NULL);

    UVMapSetOffset(uvmap, du, dv);
    UVMapRender(uvmap, canvas);

    MemUnref(umap);
  } else {
    UVMapSetOffset(uvmap, du, dv);
    UVMapRender(uvmap, canvas);

    PROFILE(PixBufClear)
      PixBufClear(shades);
    PROFILE(RenderScene)
      RenderScene(scene, shades);

    PixBufSetColorMap(shades, R_("ColorMap"));
    PixBufSetBlitMode(shades, BLIT_COLOR_MAP);

    PixBufBlit(canvas, 0, 0, shades, NULL);
  }

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

    if (event == LOOP_NEXT)
      EffectNum = (EffectNum + 1) % 2;
    if (event == LOOP_PREV)
      EffectNum = (EffectNum - 1) % 2;

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
