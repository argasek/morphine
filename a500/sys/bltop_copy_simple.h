#ifndef BLTOP_NAME
#error "BLTOP: function name not defined!"
#endif

#ifndef BLTOP_SRC_BM
#error "BLTOP: source bitmap not defined!"
#endif

#ifndef BLTOP_DST_BM
#error "BLTOP: destination bitmap not defined!"
#endif

#ifndef BLTOP_DST_WIDTH
#error "BLTOP: destination width not defined!"
#endif

#ifndef BLTOP_BPLS
#error "BLTOP: number of bitplanes not defined!"
#endif

#if !defined(BLTOP_HSIZE) || !defined(BLTOP_VSIZE)
#error "BLTOP: blit size not defined!"
#else
#define BLTOP_SIZE ((BLTOP_VSIZE << 6) | (BLTOP_HSIZE >> 4))
#endif

#ifndef BLTOP_WAIT
#define BLTOP_WAIT WaitBlitter()
#endif

__regargs void BLTOP_NAME(WORD x, WORD y) {  
  APTR *src = (BLTOP_SRC_BM)->planes;
  APTR *dst = (BLTOP_DST_BM)->planes;
  LONG dst_start = ((x & ~15) >> 3) + (y * BLTOP_DST_WIDTH / 8);
  WORD dst_modulo = (BLTOP_DST_WIDTH - BLTOP_HSIZE) / 8;
  WORD src_modulo = 0;
  UWORD bltsize = BLTOP_SIZE;
  UWORD bltalwm = -1;
  WORD i;

  if (x & 15) {
    src_modulo -= 2;
    dst_modulo -= 2;
    bltsize++;
    bltalwm = 0;
  }

  BLTOP_WAIT;
  custom->bltamod = src_modulo;
  custom->bltdmod = dst_modulo;
  custom->bltcon0 = (SRCA | DEST | A_TO_D) | ((x & 15) << ASHIFTSHIFT);
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = bltalwm;
  custom->bltapt = src[0];
  custom->bltdpt = dst[0] + dst_start;
  custom->bltsize = bltsize;

  for (i = 1; i < BLTOP_BPLS; i++) {
    BLTOP_WAIT;
    custom->bltapt = src[i];
    custom->bltdpt = dst[i] + dst_start;
    custom->bltsize = bltsize;
  }
}

#undef BLTOP_NAME
#undef BLTOP_SRC_BM
#undef BLTOP_DST_BM
#undef BLTOP_DST_WIDTH
#undef BLTOP_BPLS
#undef BLTOP_HSIZE
#undef BLTOP_VSIZE
#undef BLTOP_SIZE
#undef BLTOP_WAIT
