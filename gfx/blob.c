#include "gfx/blob.h"
#include "std/memory.h"

void renderBlobQuarter(uint8_t *blobQuarterCanvas, int radius, uint8_t intensity) {
    int i, j;
    int iSquared, jSquared;
    int distance;
    int radiusSquared = radius * radius;
    float normal = (float) intensity / (float) radiusSquared;
    float value;
    int index = 0;

    for (i = radius - 1; i >= 0; i--) {
        iSquared = i * i;
        for (j = radius - 1; j >= 0; j--) {
            jSquared = j * j;

            distance = iSquared + jSquared;

            if (distance <= radiusSquared) {
                value = intensity - (distance * normal);
            } else {
                value = 0;
            }

            blobQuarterCanvas[index] = (uint8_t) value;

            index++;
        }
    }
}

void DrawBlob(PixBufT *canvas, uint16_t x, uint16_t y, uint16_t diameter) {
    uint16_t i, j;
    uint16_t remainder = diameter >> 1;
    // Radius is either equal to remainder, or greater by 1
    uint16_t radius = diameter - remainder;
    uint32_t quarterAreaSize = radius * radius;
    uint8_t *blobQuarterCanvas = MemNew(quarterAreaSize);
    uint32_t index;
    uint32_t adjuster = (radius > remainder ? 1 : 0);
    // First, we render upper left quarter of the blob
    renderBlobQuarter(blobQuarterCanvas, radius, canvas->fgColor);

    // Upper half
    index = 0;
    for (i = y; i < y + radius; i++) {
        for (j = x; j < x + radius; j++) {
            PutPixel(canvas, j, i, blobQuarterCanvas[index]);
            index++;
        }
        index -= adjuster;
        for (j = x + radius; j < x + radius + remainder; j++) {
            index--;
            PutPixel(canvas, j, i, blobQuarterCanvas[index]);
        }
        index += radius;
    }

    if (adjuster == 1) {
        index -= radius;
    }
    // Lower half
    for (i = y + radius; i < y + radius + remainder; i++) {
        index -= radius;
        for (j = x; j < x + radius; j++) {
            PutPixel(canvas, j, i, blobQuarterCanvas[index]);
            index++;
        }
        index -= adjuster;
        for (j = x + radius; j < x + radius + remainder; j++) {
            index--;
            PutPixel(canvas, j, i, blobQuarterCanvas[index]);
        }
    }

    MemUnref(blobQuarterCanvas);

}
