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
  const unsigned width = 14;
  const unsigned height = 8;
  unsigned ix, ay, by, iy;
  double x = xproj(nx, lon);
  double y = yproj(ny, lat);
  double d2fact = (double)(nx * nx) / (4.0 * M_PI * M_PI);
  ay = floor(y) - height;
  if (ay < 0) { ay = 0; }
  by = floor(y) + height;
  if (by >= ny) { by = ny - 1; }
  for (iy = ay; iy <= by; iy++) {
    double orlat = y2rlat(ny, iy);
    for (ix = floor(x) - width; ix <= floor(x) + width; ix++) {
      unsigned cx = ix % nx;
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
      printf("%3u %3u %8.2f %8.2f\n", ilat, ilon, lat, lon);
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
      printf("%3u %3u %8.2f %8.2f\n", ilat, ilon, lat, lon);
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
  //accweight(nx, ny, oary, 255);
  return bwrite(nx, ny, oary, "zweight.out");
}
