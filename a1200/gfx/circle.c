#include "gfx/pixbuf.h"
#include "gfx/circle.h"
#include "gfx/line.h"
#include "std/debug.h"

void DrawCircleBresenham(PixBufT *canvas, int x0, int y0, int diameter) {
    const int even = !(diameter & 1);
    int radius = diameter >> 1;
    int error = 1 - radius;
    int errorY = 1;
    int errorX = -2 * radius;
    int x = radius, y = 0;

    x0 += radius;
    y0 += radius;

    if (!even) {
        PutPixel(canvas, x0, y0 + radius, canvas->fgColor);
        PutPixel(canvas, x0, y0 - radius, canvas->fgColor);
        PutPixel(canvas, x0 + radius, y0, canvas->fgColor);
        PutPixel(canvas, x0 - radius, y0, canvas->fgColor);
    }

    while (y < x) {
        if (error >= 0) {
            x--;
            errorX += 2;
            error += errorX;
        }
        y++;
        errorY += 2;
        error += errorY;
        //  1 | 2
        // ---+---
        //  4 | 3

        // 3
        PutPixel(canvas, x0 + x - even, y0 + y - even, canvas->fgColor);
        PutPixel(canvas, x0 + y - even, y0 + x - even, canvas->fgColor);
        // 1
        PutPixel(canvas, x0 - x, y0 - y, canvas->fgColor);
        PutPixel(canvas, x0 - y, y0 - x, canvas->fgColor);
        // 2
        PutPixel(canvas, x0 + x - even, y0 - y, canvas->fgColor);
        PutPixel(canvas, x0 + y - even, y0 - x, canvas->fgColor);
        // 4
        PutPixel(canvas, x0 - x, y0 + y - even, canvas->fgColor);
        PutPixel(canvas, x0 - y, y0 + x - even, canvas->fgColor);
    }
}

/*
  Further speed optimizations:
    - top and bottom lines can be drawn using just PutPixel
    - horizontal lines may be drawn using simple buffer filling (width / 2 words, + 0/1 byte)
*/
void DrawDiskBresenham(PixBufT *canvas, int x0, int y0, int diameter) {
    const int even = !(diameter & 1);
    int radius = diameter >> 1;
    int error = 1 - radius;
    int errorY = 1;
    int errorX = -2 * radius;
    int x = radius, y = 0;

    x0 += radius;
    y0 += radius;

    if (!even) {
        PutPixel(canvas, x0, y0 + radius, canvas->fgColor);
        PutPixel(canvas, x0, y0 - radius, canvas->fgColor);
        DrawLineUnsafe(canvas, x0 - radius, y0 - y, x0 + radius + 1, y0 - y);
    }

    while (y < x) {
        if (error >= 0) {
            x--;
            errorX += 2;
            error += errorX;
        }
        y++;
        errorY += 2;
        error += errorY;

        // Top
        DrawLineUnsafe(canvas, x0 - y, y0 - x, x0 + y - even + 1, y0 - x);
        // Top-middle
        DrawLineUnsafe(canvas, x0 - x, y0 - y, x0 + x - even + 1, y0 - y);
        // Bottom-middle
        DrawLineUnsafe(canvas, x0 - x, y0 + y - even, x0 + x - even + 1, y0 + y - even);
        // Bottom
        DrawLineUnsafe(canvas, x0 - y, y0 + x - even, x0 + y - even + 1, y0 + x - even);
    }
}


#define PP { canvas->data[index] = canvas->fgColor; }
#define PP_ADJUST(adjustment) { canvas->data[index] = canvas->fgColor; index += adjustment; }
#define PP_OFFS_ADJUST(offs, adjustment) { canvas->data[index + offs] = canvas->fgColor; index += adjustment; }
#define PP16 { data16 = (uint16_t *) (canvas->data + index); *data16 = color16; }
#define PP16_ADJUST(adjustment) { data16 = (uint16_t *) (canvas->data + index); *data16 = color16; index += adjustment; }

void DrawCirclePrecalculated(PixBufT *canvas, int x0, int y0, int diameter) {
    size_t index = x0 + canvas->width * y0;
    switch (diameter) {
        // Single pixel
        case 1:
            PP;
            break;
        // Square
        case 2:
            PP_ADJUST(1);
            PP_ADJUST(canvas->width);
            PP_ADJUST(-1);
            PP;
            break;
        // Diamond
        case 3:
            ++index;
            PP_ADJUST(canvas->width - 1);
            PP_ADJUST(2);
            PP_ADJUST(canvas->width - 1);
            PP;
            break;
        // Rounded square
        case 4:
            // Horizontal bar
            ++index;
            PP_ADJUST(1);
            PP_ADJUST(canvas->width - 2);
            // Vertical bars
            PP;
            PP_OFFS_ADJUST(3, canvas->width);
            PP;
            PP_OFFS_ADJUST(3, canvas->width + 1);
            // Horizontal bar
            PP_ADJUST(1);
            PP;
            break;
        case 5:
            // Horizontal bar
            index += 1;
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(canvas->width - 3);
            // Vertical bars
            PP;
            PP_OFFS_ADJUST(4, canvas->width);
            PP;
            PP_OFFS_ADJUST(4, canvas->width);
            PP;
            PP_OFFS_ADJUST(4, canvas->width + 1);
            // Horizontal bar
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP;
            break;
        case 6:
            // Horizontal bar
            ++index;
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(canvas->width - 4);
            // Vertical bars
            PP;
            PP_OFFS_ADJUST(5, canvas->width);
            PP;
            PP_OFFS_ADJUST(5, canvas->width);
            PP;
            PP_OFFS_ADJUST(5, canvas->width);
            PP;
            PP_OFFS_ADJUST(5, canvas->width + 1);
            // Horizontal bar
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP;
            break;
        case 7:
            // Top
            index += 2;
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(canvas->width - 3);
            // Middle
            PP_ADJUST(4);
            PP_ADJUST(canvas->width - 5);
            PP_ADJUST(6);
            PP_ADJUST(canvas->width - 6);
            PP_ADJUST(6);
            PP_ADJUST(canvas->width - 6);
            PP_ADJUST(6);
            PP_ADJUST(canvas->width - 5);
            // Bottom
            PP_ADJUST(4);
            PP_ADJUST(canvas->width - 3);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP;
            break;
        case 8:
            // Top
            index += 2;
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(canvas->width - 4);
            // Middle
            PP_ADJUST(5);
            PP_ADJUST(canvas->width - 6);
            PP_ADJUST(7);
            PP_ADJUST(canvas->width - 7);
            PP_ADJUST(7);
            PP_ADJUST(canvas->width - 7);
            PP_ADJUST(7);
            PP_ADJUST(canvas->width - 7);
            PP_ADJUST(7);
            PP_ADJUST(canvas->width - 6);
            // Bottom
            PP_ADJUST(5);
            PP_ADJUST(canvas->width - 4);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP;
            break;
        case 9:
            // Top
            index += 2;
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(canvas->width - 5);
            // Middle
            PP_ADJUST(6);
            PP_ADJUST(canvas->width - 7);
            PP_ADJUST(8);
            PP_ADJUST(canvas->width - 8);
            PP_ADJUST(8);
            PP_ADJUST(canvas->width - 8);
            PP_ADJUST(8);
            PP_ADJUST(canvas->width - 8);
            PP_ADJUST(8);
            PP_ADJUST(canvas->width - 8);
            PP_ADJUST(8);
            PP_ADJUST(canvas->width - 7);
            // Bottom
            PP_ADJUST(6);
            PP_ADJUST(canvas->width - 5);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP;
            break;
    }
}

void DrawDiskPrecalculated(PixBufT *canvas, int x0, int y0, int diameter) {
    size_t index = x0 + canvas->width * y0;
    uint16_t *data16;
    uint16_t color16 = (canvas->fgColor << 8) + canvas->fgColor;
    switch (diameter) {
        // Single pixel
        case 1:
            PP;
            break;
        // Square
        case 2:
            PP16_ADJUST(canvas->width);
            PP16;
            break;
        // Diamond
        case 3:
            ++index;
            PP_ADJUST(canvas->width - 1);
            // TODO: optimize?
            PP_ADJUST(1);
            PP_ADJUST(1);
            PP_ADJUST(canvas->width - 1);
            PP;
            break;
        // Rounded square
        case 4:
            // Top
            ++index;
            PP16_ADJUST(canvas->width - 1);
            // Middle
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 1);
            // Bottom
            PP16;
            break;
        case 5:
            //Top
            ++index;
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 3);
            // Middle
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 4);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 4);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 3);
            // Bottom
            PP16_ADJUST(2);
            PP;
            break;
        case 6:
            // Top
            ++index;
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 3);
            // Middle
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 4);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 4);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 4);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 3);
            // Bottom
            PP16_ADJUST(2);
            PP16;
            break;
        case 7:
            // Top
            index += 2;
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 3);
            // Top 5
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 5);
            // Middle
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 6);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 6);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 5);

            // Bottom 5
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 3);
            // Bottom
            PP16_ADJUST(2);
            PP;

            break;
        case 8:
            // Top
            index += 2;
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 3);
            // Middle

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 5);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 6);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 6);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 6);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 5);

            // Bottom
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(canvas->width - 3);

            PP16_ADJUST(2);
            PP16;
            break;
        case 9:
            // Top
            index += 2;
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 5);
            // Middle
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 7);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 8);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 8);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 8);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 8);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 7);

            // Bottom
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP_ADJUST(canvas->width - 5);

            PP16_ADJUST(2);
            PP16_ADJUST(2);
            PP;
            break;
    }
}

#undef PP
#undef PP_ADJUST
#undef PP_OFFS_ADJUST
#undef PP16
#undef PP16_ADJUST

void DrawCircle(PixBufT *canvas, int x, int y, int diameter) {
    ASSERT(canvas->type == PIXBUF_GRAY || canvas->type == PIXBUF_CLUT, "Canvas type (%d) not supported", canvas->type);
    if (diameter < 1) return;
    // If circle has a diameter smaller than 10px, we want to take care of its shape
    if (diameter >= 10) {
        DrawCircleBresenham(canvas, x, y, diameter);
    } else {
        DrawCirclePrecalculated(canvas, x, y, diameter);
    }
}

void DrawDisk(PixBufT *canvas, int x, int y, int diameter) {
    ASSERT(canvas->type == PIXBUF_GRAY || canvas->type == PIXBUF_CLUT, "Canvas type (%d) not supported", canvas->type);
    if (diameter < 1) return;
    // If disk has a diameter smaller than 10px, we want to take care of its shape
    if (diameter >= 10) {
        DrawDiskBresenham(canvas, x, y, diameter);
    } else {
        DrawDiskPrecalculated(canvas, x, y, diameter);
    }
}
