/* Smooth Bresenham, template-based */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef unsigned long PIXEL;
inline unsigned long average(unsigned long a, unsigned long b);

//template<class PIXEL>
void ScaleLineAvg(PIXEL *Target, PIXEL *Source, int SrcWidth, int TgtWidth)
{
  int NumPixels = TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int Mid = TgtWidth / 2;
  int E = 0;
  int skip;
  PIXEL p;

  skip = (TgtWidth < SrcWidth) ? 0 : TgtWidth / (2*SrcWidth) + 1;
  NumPixels -= skip;

  while (NumPixels-- > 0) {
    p = *Source;
    if (E >= Mid)
      p = average(p, *(Source+1));
    *Target++ = p;
    Source += IntPart;
    E += FractPart;
    if (E >= TgtWidth) {
      E -= TgtWidth;
      Source++;
    } /* if */
  } /* while */
  while (skip-- > 0)
    *Target++ = *Source;
}



/*  Smooth 2D scaling */

//template<class PIXEL>
void ScaleRectAvg(PIXEL *Target, PIXEL *Source, int SrcWidth, int SrcHeight,
                  int TgtWidth, int TgtHeight)
{
  int NumPixels = TgtHeight;
  int IntPart = (SrcHeight / TgtHeight) * SrcWidth;
  int FractPart = SrcHeight % TgtHeight;
  int Mid = TgtHeight / 2;
  int E = 0;
  int skip;
  PIXEL *ScanLine, *ScanLineAhead;
  PIXEL *PrevSource = NULL;
  PIXEL *PrevSourceAhead = NULL;

  skip = (TgtHeight < SrcHeight) ? 0 : TgtHeight / (2*SrcHeight) + 1;
  NumPixels -= skip;

  ScanLine = (PIXEL *)malloc(TgtWidth*sizeof(PIXEL));
  ScanLineAhead = (PIXEL *)malloc(TgtWidth*sizeof(PIXEL));

  while (NumPixels-- > 0) {
    if (Source != PrevSource) {
      if (Source == PrevSourceAhead) {
        /* the next scan line has already been scaled and stored in
         * ScanLineAhead; swap the buffers that ScanLine and ScanLineAhead
         * point to
         */
        PIXEL *tmp = ScanLine;
        ScanLine = ScanLineAhead;
        ScanLineAhead = tmp;
      } else {
        ScaleLineAvg(ScanLine, Source, SrcWidth, TgtWidth);
      } /* if */
      PrevSource = Source;
    } /* if */
    if (E >= Mid && PrevSourceAhead != Source+SrcWidth) {
      int x;
      ScaleLineAvg(ScanLineAhead, Source+SrcWidth, SrcWidth, TgtWidth);
      for (x = 0; x < TgtWidth; x++)
        ScanLine[x] = average(ScanLine[x], ScanLineAhead[x]);
      PrevSourceAhead = Source + SrcWidth;
    } /* if */
    memcpy(Target, ScanLine, TgtWidth*sizeof(PIXEL));
    Target += TgtWidth;
    Source += IntPart;
    E += FractPart;
    if (E >= TgtHeight) {
      E -= TgtHeight;
      Source += SrcWidth;
    } /* if */
  } /* while */

  if (skip > 0 && Source != PrevSource)
    ScaleLineAvg(ScanLine, Source, SrcWidth, TgtWidth);
  while (skip-- > 0) {
    memcpy(Target, ScanLine, TgtWidth*sizeof(PIXEL));
    Target += TgtWidth;
  } /* while */

  free(ScanLine);
  free(ScanLineAhead);
}




/* Averaging pixel values */

/* gray scale */
inline unsigned char average(unsigned char a, unsigned char b)
{
  return (unsigned char)( ((int)a + (int)b) >> 1);
}

/* 24-bit RGB (a pixel is packed in a 32-bit integer) */
inline unsigned long average(unsigned long a, unsigned long b)
{
  return ((a & 0xfefefeffUL) + (b & 0xfefefeffUL)) >> 1;
}

/* 16-bit HiColor (565 format) */
inline unsigned short average(unsigned short a, unsigned short b)
{
  if (a == b) {
    return a;
  } else {
    unsigned short mask = ~ (((a | b) & 0x0410) << 1);
    return ((a & mask) + (b & mask)) >> 1;
  } /* if */
}

/* palette-indexed (256 colour) pixels */
signed char average_table[65536];     /* precomputed */
inline signed char average(signed char a, signed char b)
{
  return average_table[((int)(unsigned char)a << 8) + (int)(unsigned char)b];
}

typedef struct __s_RGB {
  unsigned char r, g, b;
} RGB;

/*void MakeAverageTable(signed char *table, RGB *palette)
{
  int x, y;
  RGB m;

  for (y = 0; y < 256; y++) {
    for (x = y; x < 256; x++) {
      m.r = (unsigned char)( ((int)palette[x].r + (int)palette[y].r) / 2);
      m.g = (unsigned char)( ((int)palette[x].g + (int)palette[y].g) / 2);
      m.b = (unsigned char)( ((int)palette[x].b + (int)palette[y].b) / 2);
      table[(y << 8) + x] = table[(x << 8) + y] = PaletteLookup(palette, &m);
    }
  }
}*/







