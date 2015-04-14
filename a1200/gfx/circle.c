#include "gfx/pixbuf.h"
#include "gfx/circle.h"
#include "gfx/line.h"
#include "std/math.h"
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

typedef struct CirclePoint {
    int x;
    int y;
    float i;
    float l; // intensity
} CirclePoint;

void DrawAntialiasedSymmetricalPixels(PixBufT *canvas, CirclePoint p1, CirclePoint p2, int x, int y, int diameter, int even) {
    uint8_t color1, color2;
    int x0;
    int y0;

    //   1 | 2 | 3 | 4
    //  ---------------
    //   5 | 6 | 7 | 8

    color1 = (uint8_t) (canvas->fgColor - ceil(p1.l * canvas->fgColor));
    color2 = (uint8_t) (canvas->fgColor - ceil(p2.l * canvas->fgColor));

    x0 = x - diameter;

    // 5
    PutPixel(canvas, x0 + p1.y, y + p1.x - even, color1);
    PutPixel(canvas, x0 + p2.y, y + p2.x - even, color2);
    // 1
    PutPixel(canvas, x0 + p1.y, y - p1.x, color1);
    PutPixel(canvas, x0 + p2.y, y - p2.x, color2);

    x0 = x + diameter;

    // 8
    PutPixel(canvas, x0 - p1.y - even, y + p1.x - even, color1);
    PutPixel(canvas, x0 - p2.y - even, y + p2.x - even, color2);
    // 4
    PutPixel(canvas, x0 - p1.y - even, y - p1.x, color1);
    PutPixel(canvas, x0 - p2.y - even, y - p2.x, color2);

    y0 = y - diameter;

    // 3
    PutPixel(canvas, x + p1.x - even, y0 + p1.y, color1);
    PutPixel(canvas, x + p2.x - even, y0 + p2.y, color2);
    // 2
    PutPixel(canvas, x - p1.x, y0 + p1.y, color1);
    PutPixel(canvas, x - p2.x, y0 + p2.y, color2);

    y0 = y + diameter;

    // 6
    PutPixel(canvas, x - p1.x, y0 - p1.y - even, color1);
    PutPixel(canvas, x - p2.x, y0 - p2.y - even, color2);
    // 7
    PutPixel(canvas, x + p1.x - even, y0 - p1.y - even, color1);
    PutPixel(canvas, x + p2.x - even, y0 - p2.y - even, color2);
}

// Based on:
//
// "Recursive algorithm for circle anti-aliasing"
// by Yin Liang Jia, Jing Gao & Bing Yang Li
//
void DrawCircleAntialiased(PixBufT *canvas, int x, int y, int diameter) {
    CirclePoint P, T, G, M, B;

    const int even = !(diameter & 1);
    int i, r, x0, y0;
    int ince, incne, eight;

    float r2, r8, rsqrt2, inv_r2;
    float error_adj1, error_adj2;
    float treshold1, treshold2;

    if (diameter < 4) DrawCircle(canvas, x, y, diameter);

    r = diameter >> 1;
    x0 = x + r;
    y0 = y + r;

    r2 = (float) diameter;
    r8 = (float) (diameter << 2);
    rsqrt2 = (float) M_SQRT2 * (float) r;
    inv_r2 = 1.0f / r2;
    error_adj1 = (1.0f / r8);
    error_adj2 = (3.0f / r8);

    treshold1 = 0.5f + rsqrt2;
    treshold2 = 0.5f - rsqrt2;

    //   P | T
    //  -------
    //   G | M
    //  -------
    //     | B
    //

    // Initial pixels
    G.x = 0;
    G.y = r;
    G.i = 0.0f;

    P.x = 0;
    P.y = r - 1;
    P.i = 1.0f - r2;

    ince = 1;
    incne = 4 - diameter;

    // Magic. Don't touch.
    if (r > 5) {
        eight = lroundf((float) r * M_SQRT1_2);
        if (diameter == 31 || diameter == 36) {
            eight -= 1;
        }
    } else {
        eight = (int) truncf((float) r * M_SQRT1_2);
    }

    if (!even) {
        PutPixel(canvas, x0, y0 + r, canvas->fgColor);
        PutPixel(canvas, x0, y0 - r, canvas->fgColor);
        PutPixel(canvas, x0 - r, y0, canvas->fgColor);
        PutPixel(canvas, x0 + r, y0, canvas->fgColor);
    }

    for (i = 0; i < eight; i++) {
        // Calculate coordinate of pixels
        T.x = P.x + 1;
        T.y = P.y;

        M.x = G.x + 1;
        M.y = G.y;

        B.x = G.x + 1;
        B.y = G.y + 1;

        // Calculate iM
        M.i = (float) ince + G.i;

        // We choose pixels T, M
        if (M.i <= 0.0) {
            T.i = (float) ince + P.i;

            // Calculate intensities based on (2) & (3), (4) & (5)
            T.l = (T.i * inv_r2) - (T.i <= treshold1 ? error_adj1 : error_adj2);
            M.l = -(M.i * inv_r2) + (treshold2 <= M.i ? error_adj1 : error_adj2);

            incne += 2;

            DrawAntialiasedSymmetricalPixels(canvas, T, M, x0, y0, diameter, even);

            P = T;
            G = M;

            // We choose pixels M, B
        } else {
            B.i = (float) incne + G.i;

            // Calculate intensities based on (6) & (7), (8) & (9)
            M.l = (M.i * inv_r2) - (M.i <= treshold1 ? error_adj1 : error_adj2);
            B.l = -(B.i * inv_r2) + (treshold2 <= B.i ? error_adj1 : error_adj2);

            incne += 4;

            DrawAntialiasedSymmetricalPixels(canvas, M, B, x0, y0, diameter, even);

            P = M;
            G = B;

        }
        ince += 2;
    }

}
