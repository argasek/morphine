#ifndef __GFX_CIRCLE_H__
#define __GFX_CIRCLE_H__

#include "gfx/pixbuf.h"

void DrawCircle(PixBufT *canvas, int x, int y, int diameter);
void DrawCircleAntialiased(PixBufT *canvas, int x, int y, int diameter);
void DrawDisk(PixBufT *canvas, int x, int y, int diameter);

#endif
