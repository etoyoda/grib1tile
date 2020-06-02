#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <png.h>
#include "pngout.h"

#define EPNGFATAL ENETDOWN

  int
outpng(unsigned width, unsigned height, png_bytep *ovector,
  const char *filename)
{
  const int NTEXT = 1;
  png_text txtbuf[NTEXT];
  png_structp png;
  FILE *fp;
  fp = fopen(filename, "wb");
  if (!fp) { return EOF; }
  png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!fp) { errno = EPNGFATAL; return EOF; }
  png_infop info = png_create_info_struct(png);
  if (!info) { errno = EPNGFATAL; return EOF; }
  if (setjmp(png_jmpbuf(png))) { errno = EPNGFATAL; return EOF; }
  txtbuf[0].compression = PNG_TEXT_COMPRESSION_NONE;
  txtbuf[0].key = "Software";
  txtbuf[0].text = "https://github.com/etoyoda/grib1tile";
  txtbuf[0].text_length = strlen(txtbuf[0].text);
  png_set_text(png, info, txtbuf, NTEXT);
  png_init_io(png, fp);
  png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);
  png_write_image(png, ovector);
  png_write_end(png, NULL);
  fclose(fp);
  return 0;
}

  png_bytep *
newpngbuf(unsigned width, unsigned height)
{
  png_bytep *vector;
  png_byte *buf;
  vector = malloc(height * sizeof(png_bytep));
  if (vector == NULL) { return NULL; }
  buf = malloc(width * height * 4);
  if (buf == NULL) { free(vector); return NULL; }
  for (int j = 0; j < height; j++) {
    vector[j] = buf + width * 4;
  }
  return vector;
}

  void
freepngbuf(png_bytep *vector)
{
  free(vector[0]);
  free(vector);
}

  int
outpngf(unsigned width, unsigned height, const float *ary,
  const char *filename)
{
  png_bytep *ovector;
  int r;
  /* allocate */
  ovector = newpngbuf(width, height);
  /* convert */
  for (unsigned j = 0; j < height; j++) {
    for (unsigned i = 0; i < width; i++) {
      unsigned ofs = i + j * width;
      if (isnan(ary[ofs])) {
        ovector[j][i * 4]     = 0x80;
        ovector[j][i * 4 + 1] = 0x00;
        ovector[j][i * 4 + 2] = 0x00;
        ovector[j][i * 4 + 3] = 0xFF;
      } else {
        signed long ival = ary[ofs];
        ovector[j][i * 4]     = (ival >> 16) & 0xFF;
        ovector[j][i * 4 + 1] = (ival >> 8) & 0xFF;
        ovector[j][i * 4 + 2] = ival & 0xFF;
        ovector[j][i * 4 + 3] = 0xFF;
      }
    }
  }
  /* action */
  r = outpng(width, height, ovector, filename);
  /* free */
  freepngbuf(ovector);
  return r;
}
