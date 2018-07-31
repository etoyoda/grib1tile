#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

  inline size_t
ui3(const unsigned char *buf)
{
  return (buf[0] << 16) | (buf[1] << 8) | buf[2];
}

  inline unsigned
ui2(const unsigned char *buf)
{
  return (buf[0] << 8) | buf[1];
}

  inline long
si3(const unsigned char *buf)
{
  long r;
  r = ((buf[0] & 0x7F) << 16) | (buf[1] << 8) | buf[2];
  return (buf[0] & 0x80) ? -r : r;
}

#define VLEV_SURF -3
#define VLEV_MSL -2
#define VLEV_ERR -1

  inline int
pds2vlev(const unsigned char *buf)
{
  switch (buf[9]) {
  case 1:
    return VLEV_SURF;
  case 102:
    return VLEV_MSL;
  case 100:
    return (buf[10] << 8) | buf[11];
  default:
    return VLEV_ERR;
  }
}

  inline void
pdsreftime(struct tm *v, const unsigned char *buf)
{
  v->tm_year = buf[12] + buf[24] * 100 - 2000;
  v->tm_mon = buf[13] - 1;
  v->tm_mday = buf[14];
  v->tm_hour = buf[15];
  v->tm_min = buf[16];
  v->tm_sec = 0;
  v->tm_isdst = v->tm_wday = v->tm_yday = 0;
}

  inline int
pdsftime(int *pift1, int *pift2, const unsigned char *buf)
{
  int factor;
  switch (buf[17]) {
  case 0:
    factor = 1;
    break;
  case 1:
    factor = 60;
    break;
  default:
    fprintf(stderr, "unsupported time unit %u\n", buf[17]);
    return -1;
  }
  switch (buf[20]) {
  case 0:
    *pift1 = *pift2 = buf[18] * factor;
    break;
  case 4:
    *pift1 = buf[18] * factor;
    *pift2 = buf[19] * factor;
    break;
  default:
    fprintf(stderr, "unsupported time range type %u\n", buf[20]);
    return -1;
  }
  return 0;
}

#define myassert1(test, _plusfmt, val) \
  if (!(test)) { \
    fprintf(stderr, "myassert(%s) " _plusfmt "\n", #test, val); \
    return 3u; \
  }

#define myassert3(test, _plusfmt, v1, v2, v3) \
  if (!(test)) { \
    fprintf(stderr, "myassert(%s) " _plusfmt "\n", #test, v1, v2, v3); \
    return 3u; \
  }

  unsigned
gdscheck(unsigned char *buf, unsigned igrid)
{
  const unsigned thinpat[73] = {
    73, 73, 73, 73, 73, 73, 73, 73, 72, 72,
    72, 71, 71, 71, 70, 70, 69, 69, 68, 67,
    67, 66, 65, 65, 64, 63, 62, 61, 60, 60,
    59, 58, 57, 56, 55, 54, 52, 51, 50, 49,
    48, 47, 45, 44, 43, 42, 40, 39, 38, 36,
    35, 33, 32, 30, 29, 28, 26, 25, 23, 22,
    20, 19, 17, 16, 14, 12, 11,  9,  8,  6,
    5,  3,  2 };
  unsigned n, nrows, i;
  long la1, la2, lo1, lo2;
  /* common feature */
  myassert1(buf[3] == 0, "NV=%u", buf[3]);
  myassert1(buf[5] == 0, "gridtype=%u", buf[5]);
  nrows = ui2(buf + 8);
  myassert1(nrows == 73, "%u", nrows);
  /* igrid-dependent feature */
  la1 = si3(buf + 10);
  lo1 = si3(buf + 13);
  la2 = si3(buf + 17);
  lo2 = si3(buf + 20);
  switch (igrid) {
  case 37: case 38: case 39: case 40:
    myassert1((la1 == 0), "%lu", la1);
    myassert1((la2 == 90000), "%lu", la2);
    for (i = 0; i < 73; i++) {
      unsigned ncols = ui2(buf + buf[4] + i * 2 - 1);
      myassert3((ncols == thinpat[i]), "ncols=%u thinpat[%u]=%u",
        ncols, i, thinpat[i]);
    }
    break;
  case 41: case 42: case 43: case 44:
    myassert1((la1 == -90000), "%lu", la1);
    myassert1((la2 == 0), "%lu", la2);
    for (i = 0; i < 73; i++) {
      unsigned ncols = ui2(buf + buf[4] + i * 2 - 1);
      myassert3((ncols == thinpat[i]), "ncols=%u thinpat[%u]=%u",
        ncols, i, thinpat[i]);
    }
    break;
  }
  switch (igrid) {
  case 37: case 41:
    myassert1((lo1 == 330000), "%lu", lo1);
    myassert1((lo2 == 60000), "%lu", lo2);
    break;
  case 38: case 42:
    myassert1((lo1 == 60000), "%lu", lo1);
    myassert1((lo2 == 150000), "%lu", lo2);
    break;
  case 39: case 43:
    myassert1((lo1 == 150000), "%lu", lo1);
    myassert1((lo2 == 240000), "%lu", lo2);
    break;
  case 40: case 44:
    myassert1((lo1 == 240000), "%lu", lo1);
    myassert1((lo2 == 330000), "%lu", lo2);
    break;
  }

  return 0u;
}

  unsigned
scanmsg(unsigned char *buf, size_t buflen, const char *locator)
{
  unsigned r = 0;
  size_t pdsofs = 8, pdslen, gdsofs, gdslen, bdsofs, bdslen;
  unsigned igrid, ielem;
  int ilev, ift1, ift2;
  struct tm reftime;
  pdslen = ui3(buf + pdsofs);
  if (pdslen + 8 > buflen) {
    fprintf(stderr, "PDS len=%zu goes beyond EOM %zu\n", pdslen, buflen);
    return 1;
  }
  if (buf[pdsofs + 4] != 34) {
    fprintf(stderr, "skip: originating centre %u != 34 (JMA)\n", buf[pdsofs + 4]);
    return 1;
  }
  igrid = buf[pdsofs + 6];
  if ((igrid < 37) || (igrid > 44)) {
    fprintf(stderr, "unsupported grid %u out of 37..44\n", igrid);
    return 1;
  }
  if (buf[pdsofs + 7] != 0x80) {
    fprintf(stderr, "unsupported flags 0x%zX != 0x80\n", buf[pdsofs + 7]);
    return 1;
  }
  ielem = buf[pdsofs + 8];
  ilev = pds2vlev(buf + pdsofs);
  if (ilev == VLEV_ERR) {
    fprintf(stderr, "unsupported vert level %u\n", buf[pdsofs + 9]);
    return 1;
  }
  pdsreftime(&reftime, buf + pdsofs);
  pdsftime(&ift1, &ift2, buf + pdsofs);
  {
    char rtbuf[32];
    strftime(rtbuf, sizeof rtbuf, "%Y-%m-%dT%H:%MZ", &reftime);
    fprintf(stderr, "%s: g%03u e%03u v%05d r%s f%d..%d\n", locator,
      igrid, ielem, ilev, rtbuf, ift1, ift2);
  }
  /* */
  gdsofs = pdsofs + pdslen;
  if (gdsofs + 8 > buflen) {
    fprintf(stderr, "GDS @%zu comes beyond EOM %zu\n", gdsofs, buflen);
    return 1;
  }
  gdslen = ui3(buf + gdsofs);
  r = gdscheck(buf + gdsofs, igrid);
  if (r != 0) {
    return r;
  }
  bdsofs = gdsofs + gdslen;
  if (bdsofs + 8 > buflen) {
    fprintf(stderr, "BDS @%zu comes beyond EOM %zu\n", bdsofs, buflen);
    return 1;
  }
  bdslen = ui3(buf + bdsofs);
  if (bdsofs + bdslen + 4 > buflen) {
    fprintf(stderr, "BDS @%zu+%zu goes beyond EOM %zu\n", bdsofs, bdslen, buflen);
    return 1;
  }
  

  if (memcmp(buf + bdsofs + bdslen, "7777", 4) != 0) {
    fprintf(stderr, "magic number '7777' lost\n");
    return 1;
  }
  return 0;
}

  unsigned
gdecode(FILE *fp, const char *locator)
{
  unsigned r = 0;
  unsigned char ids[4];
  size_t zr, msglen;
  unsigned char *msgbuf;
  /* section 0 IDS */
  zr = fread(ids, 1, 4, fp);
  if (zr < 4) {
    fputs("EOF in IDS", stderr);
    return 1u;
  }
  if (ids[3] != 1u) {
    fputs("Not GRIB Edition 1", stderr);
    return 1u;
  }
  msglen = ui3(ids);
  /* begin ensure malloc-free */
  msgbuf = malloc(msglen);
  if (msgbuf == NULL) {
    perror("malloc");
    return 1u;
  }
  memcpy(msgbuf+0, "GRIB", 4);
  memcpy(msgbuf+4, ids, 4);
  zr = fread(msgbuf+8, 1, msglen-8, fp);
  if (zr < msglen - 8) {
    fputs("EOF in GRIB", stderr);
    r = 1; goto freeret;
  }
  r = scanmsg(msgbuf, msglen, locator);
freeret:
  free(msgbuf);
  /* end ensure malloc-free */
  return r;
}

  unsigned
scandata(const char *fnam)
{
  FILE *fp;
  long lpos = 0;
  int c;
  int state = 0;
  unsigned r = 0;
  fp = fopen(fnam, "rb");
  if (fp == NULL) { goto error; }
  fprintf(stderr, "=== file %s ===\n", fnam);
  /* automaton to find the magic number "GRIB */
  while ((c = getc(fp)) != EOF) {
    switch (state) {
    default:
    case 0:
      if (c == 'G') { state = c; lpos = ftell(fp); } else { state = 0; }
      break;
    case 'G':
      if (c == 'R') { state = c; } else { state = 0; }
      break;
    case 'R':
      if (c == 'I') { state = c; } else { state = 0; }
      break;
    case 'I':
      if (c == 'B') {
        char locator[32];
	snprintf(locator, sizeof locator, "%-0.24s:%lu",
	  fnam + ((strlen(fnam) > 24) ? (strlen(fnam) - 24) : 0), lpos);
	r = gdecode(fp, locator);
	if (r != 0) {
	  fprintf(stderr, "%s: GRIB1 decode %u\n", locator, r);
	}
	if (r & ~1) goto klose;
      }
      state = 0;
      break;
    }
  }
klose:
  if (fclose(fp) != 0) { goto error; }
  return r;
error:
  perror(fnam);
  return 2;
}

  unsigned
scanconfig(const char *fnam)
{
  return 0;
}

  int
main(int argc, const char **argv)
{
  unsigned r;
  unsigned nconfig = 0, ndatafile = 0;
  const char **args;
  for (args = argv + 1; *args; args++) {
    if (nconfig == 0) {
      r = scanconfig(*args);
      if (r != 0) goto err;
      nconfig = 1;
    } else {
      /* 0: okay, 1: warning, other bits: fatal */
      r = scandata(*args);
      if (r & ~1) goto err;
      if (r == 0) ndatafile++;
    }
  }
  if (ndatafile == 0) {
    fprintf(stderr, "usage: %s config data ...\n", argv[0]);
    r = 2;
  }
  return r;
err:
  fprintf(stderr, "%s: exit(%u)\n", argv[0], r);
  return r;
}
