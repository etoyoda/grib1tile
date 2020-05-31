#include <stdio.h>
#include <stdlib.h>

  void
accweight(unsigned nx, unsigned ny, float oary[], int igrid)
{
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
