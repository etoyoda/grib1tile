#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pngout.h"

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

  double
rad2deg(double rad)
{
  return rad * (180.0 / M_PI);
}

  double
weight(double d2fact, double lon1, double lat1, double lon2, double lat2)
{
  double x1, x2, y1, y2, z1, z2, dsq;
  x1 = cos(lon1) * cos(lat1);
  y1 = sin(lon1) * cos(lat1);
  z1 = sin(lat1);
  x2 = cos(lon2) * cos(lat2);
  y2 = sin(lon2) * cos(lat2);
  z2 = sin(lat2);
  dsq = -(x1-x2)*(x1-x2) - (y1-y2)*(y1-y2) - (z1-z2)*(z1-z2);
  return exp(dsq * d2fact);
}

  void
accpoint(unsigned nx, unsigned ny, float oary[], double lon, double lat)
{
  signed width = 6;
  signed height = 5;
  signed ix, ay, by, iy;
  double d2fact = (double)(nx * nx) / (4.0 * M_PI * M_PI);
  double x = xproj(nx, lon);
  double y;
  if (lat <= -88.0) return;
  y = yproj(ny, lat);
  if (y < -0.0) return;
  if ((lat >= -40.0) && (lat <= 40.0)) { width = height = 2; }
  ay = floor(y) - height;
  if (ay < 0) { ay = 0; }
  by = floor(y) + height;
  if (by >= ny) { by = ny - 1; }
  for (iy = ay; iy <= by; iy++) {
    double orlat = y2rlat(ny, iy);
    for (ix = floor(x) - width; ix <= floor(x) + width; ix++) {
      signed cx = ix % nx;
      double orlon = x2rlon(nx, cx);
      unsigned oofs = cx + nx * iy;
      oary[oofs] += weight(d2fact, orlon, orlat, deg2rad(lon), deg2rad(lat));
    }
  }
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
      accpoint(nx, ny, oary, lon, lat);
    }
  }
}

const unsigned thinpat[73] = {
  73, 73, 73, 73, 73, 73, 73, 73, 72, 72,
  72, 71, 71, 71, 70, 70, 69, 69, 68, 67,
  67, 66, 65, 65, 64, 63, 62, 61, 60, 60,
  59, 58, 57, 56, 55, 54, 52, 51, 50, 49,
  48, 47, 45, 44, 43, 42, 40, 39, 38, 36,
  35, 33, 32, 30, 29, 28, 26, 25, 23, 22,
  20, 19, 17, 16, 14, 12, 11,  9,  8,  6,
  5,  3,  2 };

  void
accthin(unsigned nx, unsigned ny, float oary[],
  double lon1, double lon2,
  double lat1, double lat2, unsigned nlat)
{
  unsigned ilon, ilat, nlon;
  for (ilat = 0; ilat < nlat; ilat++) {
    double lat = lat1 + ((lat2 - lat1) * ilat) / (nlat - 1);
    nlon = (lat1 == 0.0) ? (thinpat[ilat]) : (thinpat[nlat - ilat - 1]);
    for (ilon = 0; ilon < nlon; ilon++) {
      double lon = lon1 + ((lon2 - lon1) * ilon) / (nlon - 1);
      accpoint(nx, ny, oary, lon, lat);
    }
  }
}

  void
accweight(unsigned nx, unsigned ny, float oary[], int igrid)
{
  switch (igrid) {
  case 37:
    accthin(nx, ny, oary, -30.0, 60.0, 0.0, 90.0, 73u);
  break;
  case 38:
    accthin(nx, ny, oary, 60.0, 150.0, 0.0, 90.0, 73u);
  break;
  case 39:
    accthin(nx, ny, oary, 150.0, 240.0, 0.0, 90.0, 73u);
  break;
  case 40:
    accthin(nx, ny, oary, 240.0, 330.0, 0.0, 90.0, 73u);
  break;
  case 41:
    accthin(nx, ny, oary, -30.0, 60.0, -90.0, 0.0, 73u);
  break;
  case 42:
    accthin(nx, ny, oary, 60.0, 150.0, -90.0, 0.0, 73u);
  break;
  case 43:
    accthin(nx, ny, oary, 150.0, 240.0, -90.0, 0.0, 73u);
  break;
  case 44:
    accthin(nx, ny, oary, 240.0, 330.0, -90.0, 0.0, 73u);
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
#if 0
  unsigned nw;
  FILE *ofp;
  char cmd[256];
  int r;
  ofp = fopen("ztmp.gray", "wb");
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
  r = snprintf(cmd, sizeof cmd,
  "convert -size %ux%u -depth 32 -define "
  "quantum:format=floating-point ztmp.gray %s", nx, ny, ofn);
  if (r > sizeof cmd) {
    fputs("too long filename\n", stderr);
    return 8;
  }
  r = system(cmd);
  if (r == -1) {
    perror(cmd);
    return 16;
  }
  return WEXITSTATUS(r);
#else
  return outpngf(nx, ny, oary, ofn);
#endif
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
  return bwrite(nx, ny, oary, "zweight.png");
}
