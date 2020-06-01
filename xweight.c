#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/** 経度 lon （度）から出力格子位置 x （端数あり）を計算する */
/* 注意: x 方向の両端を 0°E にするような真似はしない */
  double
xproj(unsigned nx, double lon)
{
  if (lon < 0.0) { lon += 360.0; }
  if (lon > 360.0) { lon -= 360.0; }
  return lon * nx / 360.0;
}

/** 経度 lat （度）から出力格子位置 y （端数あり）を計算する */
  double
yproj(unsigned ny, double lat)
{
  double y;
  y = 0.5 - log(tan(M_PI / 4 + M_PI * lat / 360.0)) / (M_PI * 2.0);
  if (y < 0.0) { y = 0.0; }
  if (y > 1.0) { y = 1.0; }
  return y * (ny - 1);
}

/** 出力格子位置 x （端数あり）から経度（ラジアン）を計算する */
  double
x2rlon(unsigned nx, double x)
{
  return (x / nx) * M_PI * 2.0;
}

/** 出力格子位置 y （端数あり）から緯度（ラジアン）を計算する */
  double
y2rlat(unsigned ny, double y)
{
  double yn = (0.5 - y / (ny - 1)) * 2.0 * M_PI;
  return 2.0 * atan(exp(yn)) - M_PI_2;
}

/** 角度（度）をラジアンに換算 */
  double
deg2rad(double deg)
{
  return deg * (M_PI / 180.0);
}

  void
accregular(unsigned nx, unsigned ny, float oary[],
  double lon1, double lon2, unsigned nlon,
  double lat1, double lat2, unsigned nlat)
{
  unsigned ilon, ilat;
  for (ilat = 0; ilat < nlat; ilat++) {
    double lat = lat1 + ((lat2 - lat1) * ilat) / (nlat - 1);
    for (ilon = 0; ilon < nlon; ilon++) {
      double lon = lon1 + ((lon2 - lon1) * ilon) / (nlon - 1);
      double x = xproj(nx, lon);
      double y = yproj(ny, lat);
      printf("%3u %3u %8.2f %8.2f %8.2f %8.2f %8.2f\n", ilat, ilon, lat, lon, x, y, y2rlat(ny, y) / M_PI * 180.0);
    }
  }
}

  void
accweight(unsigned nx, unsigned ny, float oary[], int igrid)
{
  switch (igrid) {
  case 37:
  break;
  case 44:
  break;
  case 255:
    accregular(nx, ny, oary, 60.0, 200.0, 113u, 60.0, -20.0, 65u);
  break;
  default:
    fprintf(stderr, "invalid igrid=%d\n", igrid);
  break;
  }
}

  int
bwrite(unsigned nx, unsigned ny, float oary[], const char *ofn)
{
  unsigned nw;
  FILE *ofp;
  ofp = fopen(ofn, "wb");
  if (ofp == NULL) {
    perror(ofn);
    return 16;
  }
  nw = fwrite(oary, sizeof oary[0], nx * ny, ofp);
  if (nw < nx * ny) {
    perror(ofn);
    return 16;
  }
  fclose(ofp);
  return 0;
}

  int
main(int argc, char **argv)
{
  unsigned nx = 288;
  unsigned ny = 289;
  float *oary;
  unsigned i;
  oary = malloc(sizeof(float) * nx * ny);
  if (oary == NULL) {
    perror("malloc");
    return 12;
  }
  for (i = 0; i < nx * ny; i++) {
    oary[i] = 0.0f;
  }
  for (i = 37; i <= 44; i++) {
    accweight(nx, ny, oary, i);
  }
  accweight(nx, ny, oary, 255);
  return bwrite(nx, ny, oary, "zweight.out");
}
