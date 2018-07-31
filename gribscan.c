#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

  size_t
ui3(const unsigned char *buf)
{
  return (buf[0] << 16) | (buf[1] << 8) | buf[2];
}

  unsigned
ui2(const unsigned char *buf)
{
  return (buf[0] << 8) | buf[1];
}

  long
si3(const unsigned char *buf)
{
  long r;
  r = ((buf[0] & 0x7Fu) << 16) | (buf[1] << 8) | buf[2];
  return (buf[0] & 0x80u) ? -r : r;
}

  int
si2(const unsigned char *buf)
{
  long r;
  r = ((buf[0] & 0x7Fu) << 8) | buf[1];
  return (buf[0] & 0x80u) ? -r : r;
}

  float
mfloat(const unsigned char *buf)
{
  unsigned bsign, exp;
  unsigned long mant;
  float r;
  bsign = buf[0] >> 7;
  exp = ((signed)(buf[0] & 0x7Fu) - 64) * 4 - 24;
  mant = ui3(buf + 1);
  r = ldexpf((float)mant, exp);
  return bsign ? -r : r;
}

  unsigned
getbits(const unsigned char *buf, size_t bitofs, size_t nbits)
{
  if (nbits == 7u) {
    switch (bitofs) {
    case 0: return buf[0] >> 1;
    case 1: return buf[0] & 0x7Fu;
    case 2: return (buf[0] << 1) & 0x7Fu | buf[1] >> 7;
    case 3: return (buf[0] << 2) & 0x7Fu | buf[1] >> 6;
    case 4: return (buf[0] << 3) & 0x7Fu | buf[1] >> 5;
    case 5: return (buf[0] << 4) & 0x7Fu | buf[1] >> 4;
    case 6: return (buf[0] << 5) & 0x7Fu | buf[1] >> 3;
    case 7: return (buf[0] << 6) & 0x7Fu | buf[1] >> 2;
    }
  } else if (nbits == 8u) {
    switch (bitofs) {
    case 0: return buf[0];
    case 1: return (buf[0] << 1) & 0xFFu | buf[1] >> 7;
    case 2: return (buf[0] << 2) & 0xFFu | buf[1] >> 6;
    case 3: return (buf[0] << 3) & 0xFFu | buf[1] >> 5;
    case 4: return (buf[0] << 4) & 0xFFu | buf[1] >> 4;
    case 5: return (buf[0] << 5) & 0xFFu | buf[1] >> 3;
    case 6: return (buf[0] << 6) & 0xFFu | buf[1] >> 2;
    case 7: return (buf[0] << 7) & 0xFFu | buf[1] >> 1;
    }
  } else if (nbits == 10u) {
    switch (bitofs) {
    case 0: return (buf[0] << 2) & 0x3FFu | buf[1] >> 6;
    case 1: return (buf[0] << 3) & 0x3FFu | buf[1] >> 5;
    case 2: return (buf[0] << 4) & 0x3FFu | buf[1] >> 4;
    case 3: return (buf[0] << 5) & 0x3FFu | buf[1] >> 3;
    case 4: return (buf[0] << 6) & 0x3FFu | buf[1] >> 2;
    case 5: return (buf[0] << 7) & 0x3FFu | buf[1] >> 1;
    case 6: return (buf[0] << 8) & 0x3FFu | buf[1];
    case 7: return (buf[0] << 9) & 0x3FFu | buf[1] << 1 | buf[2] >> 7;
    }
  } else if (nbits == 12u) {
    switch (bitofs) {
    case 0: return (buf[0] <<  4) & 0xFFFu | buf[1] >> 4;
    case 1: return (buf[0] <<  5) & 0xFFFu | buf[1] >> 3;
    case 2: return (buf[0] <<  6) & 0xFFFu | buf[1] >> 2;
    case 3: return (buf[0] <<  7) & 0xFFFu | buf[1] >> 1;
    case 4: return (buf[0] <<  8) & 0xFFFu | buf[1];
    case 5: return (buf[0] <<  9) & 0xFFFu | buf[1] << 1 | buf[2] >> 7;
    case 6: return (buf[0] << 10) & 0xFFFu | buf[1] << 2 | buf[2] >> 6;
    case 7: return (buf[0] << 11) & 0xFFFu | buf[1] << 3 | buf[2] >> 5;
    }
  } else {
    return (buf[0] << (nbits + bitofs - 8u)) & ((1u << nbits) - 1u)
      | (nbits + bitofs < 16u
	  ? buf[1] >> (16u - nbits - bitofs)
	  : buf[1] << (nbits + bitofs - 16u)
        )
      | (nbits + bitofs < 24u
          ? buf[2] >> (24u - nbits - bitofs)
	  : buf[2] << (nbits + bitofs - 24u)
	)
      | buf[3] >> (32u - nbits - bitofs)
      ;
  } 
}

  unsigned
unpackbits(const unsigned char *buf, size_t nbits, size_t pos)
{
  size_t byteofs = (nbits * pos) / 8u;
  size_t bitofs = (nbits * pos) % 8u;
  return getbits(buf + byteofs, bitofs, nbits);
}

/* --- memory layout ---
 * dcdbuf array := n_ftime x (layers data)
 * layers data := 
 */

unsigned *dcdbuf = NULL;

  unsigned
scanconfig(const char *fnam)
{
  
  return 0;
}

#define VLEV_SURF 0
#define VLEV_MSL 0
#define VLEV_ERR -1

  int
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

  void
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

  const char *
showtime(char *buf, size_t size, const struct tm *t)
{
    strftime(buf, size, "%Y-%m-%dT%H:%MZ", t);
    return buf;
}

  int
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

#define MYASSERT1(test, _plusfmt, val) \
  if (!(test)) { \
    fprintf(stderr, "assert(%s) " _plusfmt "\n", #test, val); \
    return 3u; \
  }

#define MYASSERT3(test, _plusfmt, v1, v2, v3) \
  if (!(test)) { \
    fprintf(stderr, "assert(%s) " _plusfmt "\n", #test, v1, v2, v3); \
    return 3u; \
  }

  unsigned
gdscheck(unsigned char *gds, unsigned igrid)
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
  MYASSERT1(gds[3] == 0, "NV=%u", gds[3]);
  MYASSERT1(gds[5] == 0, "gridtype=%u", gds[5]);
  nrows = ui2(gds + 8);
  MYASSERT1(nrows == 73u, "%u", nrows);
  /* igrid-dependent feature */
  la1 = si3(gds + 10);
  lo1 = si3(gds + 13);
  la2 = si3(gds + 17);
  lo2 = si3(gds + 20);
  switch (igrid) {
  case 37: case 38: case 39: case 40:
    MYASSERT1((la1 == 0), "%lu", la1);
    MYASSERT1((la2 == 90000), "%lu", la2);
    for (i = 0; i < 73; i++) {
      unsigned ncols = ui2(gds + gds[4] + i * 2 - 1);
      MYASSERT3((ncols == thinpat[i]), "ncols=%u thinpatN[%u]=%u",
        ncols, i, thinpat[i]);
    }
    break;
  case 41: case 42: case 43: case 44:
    MYASSERT1((la1 == -90000), "%lu", la1);
    MYASSERT1((la2 == 0), "%lu", la2);
    for (i = 0; i < 73; i++) {
      unsigned ncols = ui2(gds + gds[4] + i * 2 - 1);
      MYASSERT3((ncols == thinpat[72-i]), "ncols=%u thinpatS[%u]=%u",
        ncols, i, thinpat[72-i]);
    }
    break;
  }
  switch (igrid) {
  case 37: case 41:
    MYASSERT1((lo1 == 330000), "%lu", lo1);
    MYASSERT1((lo2 == 60000), "%lu", lo2);
    break;
  case 38: case 42:
    MYASSERT1((lo1 == 60000), "%lu", lo1);
    MYASSERT1((lo2 == 150000), "%lu", lo2);
    break;
  case 39: case 43:
    MYASSERT1((lo1 == 150000), "%lu", lo1);
    MYASSERT1((lo2 == 240000), "%lu", lo2);
    break;
  case 40: case 44:
    MYASSERT1((lo1 == 240000), "%lu", lo1);
    MYASSERT1((lo2 == 330000), "%lu", lo2);
    break;
  }

  return 0u;
}

#define NPTS_MSG 3447
#define NPTS_PLANE 26704

#define WEAK_ASSERT1(test, _plusfmt, val) \
  if (!(test)) { \
    fprintf(stderr, "assert(%s) " _plusfmt "\n", #test, val); \
    return 1u; \
  }

  unsigned
bdsdecode(const unsigned char *bds, size_t buflen, unsigned igrid, unsigned iparm,
  int d_scale)
{
  int e_scale = si2(bds + 4);
  float refval = mfloat(bds + 6);
  unsigned depth = bds[10];
  unsigned blankbits = bds[3] & 0xFu;
  double dfactor = pow(10.0, -d_scale);
  float maxval = refval + ((1 << depth) - 1) * ldexpf(1.0f, e_scale);
  int i, j;
  if (iparm == 2) { dfactor *= 0.01; };
  fprintf(stderr, "pa%03u Es%03d dp%03u min%-9.6g max%-9.6g\n",
    iparm, e_scale, depth, refval * dfactor, maxval * dfactor);
  MYASSERT3(depth * NPTS_MSG + blankbits + 88u == buflen * 8,
    "depth=%u blankbits=%u buflen=%zu", depth, blankbits, buflen);
  for (i = 0; i < NPTS_MSG; i++) {
    unsigned y;
    y = unpackbits(bds + 11u, depth, i);
    fprintf(stderr, " %u", y);
  }
  fprintf(stderr, "\n");
  return 0;
}

  unsigned
scanmsg(unsigned char *buf, size_t buflen, const char *locator)
{
  unsigned r = 0;
  size_t pdsofs = 8, pdslen, gdsofs, gdslen, bdsofs, bdslen;
  unsigned igrid, iparm;
  int ilev, ift1, ift2, d_scale;
  struct tm reftime;
  char rtbuf[32];
  pdslen = ui3(buf + pdsofs);
  MYASSERT1((pdslen + 8 < buflen), "pdslen=%zu", pdslen);
  WEAK_ASSERT1((buf[pdsofs + 4] == 34), "origcenter=%u", buf[pdsofs + 4]);
  igrid = buf[pdsofs + 6];
  WEAK_ASSERT1((igrid >= 37) && (igrid <= 44), "gridtype=%u", igrid);
  WEAK_ASSERT1((buf[pdsofs + 7] == 0x80u), "flags=%#X", buf[pdsofs + 7]);
  iparm = buf[pdsofs + 8];
  ilev = pds2vlev(buf + pdsofs);
  if (ilev == VLEV_ERR) {
    fprintf(stderr, "unsupported vert level %u\n", buf[pdsofs + 9]);
    return 1;
  }
  pdsreftime(&reftime, buf + pdsofs);
  pdsftime(&ift1, &ift2, buf + pdsofs);
  d_scale = si2(buf + pdsofs + 26);
  fprintf(stderr, "%s: g%03u p%03u v%04d r%s f%03d..%03d\n", locator,
    igrid, iparm, ilev, showtime(rtbuf, sizeof rtbuf, &reftime), ift1/60, ift2/60);
  /* */
  gdsofs = pdsofs + pdslen;
  MYASSERT1((gdsofs + 8 < buflen), "gdsofs=%zu", gdsofs);
  gdslen = ui3(buf + gdsofs);
  /* may return 1 */
  r = gdscheck(buf + gdsofs, igrid);
  if (r != 0) {
    return r;
  }
  bdsofs = gdsofs + gdslen;
  MYASSERT1((bdsofs + 8 < buflen), "bdspos=%zu", bdsofs);
  bdslen = ui3(buf + bdsofs);
  MYASSERT3((bdsofs + bdslen + 4 <= buflen), "bdsofs=%zu, bdslen=%zu, buflen=%zu",
    bdsofs, bdslen, buflen);
  MYASSERT1(memcmp(buf + bdsofs + bdslen, "7777", 4) == 0, "endpos=%zu", bdsofs + bdslen);
  r = bdsdecode(buf + bdsofs, bdslen, igrid, iparm, d_scale);
  return r;
}

/* returns: 0=okay, 1=just warning, 2-or-more=stop */
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
    return 3u;
  }
  if (ids[3] != 1u) {
    fputs("Not GRIB Edition 1", stderr);
    return 1u;
  }
  msglen = ui3(ids);
  msgbuf = malloc(msglen);
  if (msgbuf == NULL) {
    perror("malloc");
    return 1u;
  }
  /* --- begin ensure malloc-free --- */
  memcpy(msgbuf+0, "GRIB", 4);
  memcpy(msgbuf+4, ids, 4);
  zr = fread(msgbuf+8, 1, msglen-8, fp);
  if (zr < msglen - 8) {
    fputs("EOF in GRIB", stderr);
    r = 1; goto free_and_return;
  }
  r = scanmsg(msgbuf, msglen, locator);
free_and_return:
  free(msgbuf);
  /* --- end ensure malloc-free --- */
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
	snprintf(locator, sizeof locator, "%-.24s:%lu",
	  fnam + ((strlen(fnam) > 24) ? (strlen(fnam) - 24) : 0), lpos);
	r = gdecode(fp, locator);
	if (r != 0) {
	  fprintf(stderr, "%s: GRIB1 decode %u\n", locator, r);
	}
	/* return code 1 is just warning, but 2 or more stops reading of the file */
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
