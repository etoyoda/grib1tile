#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <png.h>

#define EPNGFATAL ENETDOWN

struct georefimg {
  /* --- members to hold image --- */
  int   		img_width;
  int   		img_height;
  /* actual type <png_bytep *>, here <void *> is used to conseal png.h dependency. */
  void			**img_vector;
  /* --- members to describe map projection --- */
//  enum projtype		img_projtype;
  /* latitude of the first grid point */
  double		img_ba;
  /* latitude of the last grid point */
  double		img_bz;
  /* longitude of the first grid point */
  double		img_la;
  /* longitude of the last grid point */
  double		img_lz;
  /* longitude of center of projection */
  double		img_lc;
  double		img_cw;
  double		img_ch;
  double		img_sw;
  double		img_sh;
  /* --- pointer to chain to the next image --- */
  struct georefimg	*img_next;
//  enum outfilter	img_of;
};

struct outparams {
  unsigned z;  /* zoom level */
  unsigned xa;  /* global x index of first pixel (= 256 xfirst) */
  unsigned xz;  /* global x index of last pixel (= 256 xlast + 255) */
  unsigned ya;  /* global y index of first pixel (= 256 yfirst) */
  unsigned yz;  /* global y index of last pixel (= 256 ylast + 255) */
  const char *filename;
};

  int
writeimg(const struct outparams *op, png_bytep *ovector)
{
  FILE *fp = fopen(op->filename, "wb");
  if (!fp) { return EOF; }
  png_structp png = png_create_write_struct(
    PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!fp) { errno = EPNGFATAL; return EOF; }
  png_infop info = png_create_info_struct(png);
  if (!info) { errno = EPNGFATAL; return EOF; }

  if (setjmp(png_jmpbuf(png))) { errno = EPNGFATAL; return EOF; }

  {
    const int NTEXT = 1;
    png_text txtbuf[NTEXT];
    txtbuf[0].compression = PNG_TEXT_COMPRESSION_NONE;
    txtbuf[0].key = "Software";
    txtbuf[0].text = "https://github.com/etoyoda/grib1tile";
    txtbuf[0].text_length = strlen(txtbuf[0].text);
    png_set_text(png, info, txtbuf, NTEXT);
  }

  png_init_io(png, fp);

  unsigned owidth = op->xz - op->xa + 1; 
  unsigned oheight = op->yz - op->ya + 1;
  png_set_IHDR(png, info, owidth, oheight, 8, PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);
  png_write_image(png, ovector);
  png_write_end(png, NULL);
  fclose(fp);
  
  return 0;
}

  unsigned long
rgbint(const png_byte *pix)
{
  return
    ((unsigned char)pix[0] << 16) |
    ((unsigned char)pix[1] << 8) |
     (unsigned char)pix[2]
    ;
}

