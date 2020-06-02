#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <png.h>

#define EPNGFATAL ENETDOWN

  int
writepng(unsigned width, unsigned height, png_bytep *ovector,
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

