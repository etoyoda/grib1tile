#include <stdio.h>
#include <stdlib.h>
#include <string.h>

  inline size_t
ui3(const unsigned char *buf)
{
  return (buf[0] << 16) | (buf[1] << 8) | buf[2];
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

  unsigned
scanmsg(unsigned char *buf, size_t buflen)
{
  unsigned r = 0;
  size_t pdsofs = 8, pdslen, gdsofs, gdslen, bdsofs, bdslen;
  unsigned igrid, ielem;
  int ilev;
  pdslen = ui3(buf + pdsofs);
  if (pdslen + 8 > buflen) {
    fprintf(stderr, "PDS len=%zu goes beyond EOM %zu\n", pdslen, buflen);
    return 1;
  }
  if (buf[pdsofs + 4] != 34) {
    fprintf(stderr, "skip: originating centre %u != 34 (JMA)\n", buf[pdsofs + 4]);
    return 0;
  }
  igrid = buf[pdsofs + 6];
  if ((igrid < 37) || (igrid > 44)) {
    fprintf(stderr, "unsupported grid %u out of 37..44\n", igrid);
    return 0;
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
  fprintf(stderr, "g%03u e%03u v%05d\n", igrid, ielem, ilev);
  /* */
  gdsofs = pdsofs + pdslen;
  if (gdsofs + 8 > buflen) {
    fprintf(stderr, "GDS @%zu comes beyond EOM %zu\n", gdsofs, buflen);
    return 1;
  }
  gdslen = ui3(buf + gdsofs);
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
gdecode(FILE *fp)
{
  unsigned r = 0;
  unsigned char ids[4];
  size_t zr, msglen;
  unsigned char *msgbuf;
  /* section 0 IDS */
  zr = fread(ids, 1, 4, fp);
  if (zr < 4) {
    fputs("EOF in IDS", stderr); r = 1; goto ret;
  }
  if (ids[3] != 1u) {
    fputs("Not GRIB Edition 1", stderr); r = 1; goto ret;
  }
  msglen = ui3(ids);
  /* begin ensure malloc-free */
  msgbuf = malloc(msglen);
  if (msgbuf == NULL) {
    perror("malloc"); r = 1; goto ret;
  }
  memcpy(msgbuf+0, "GRIB", 4);
  memcpy(msgbuf+4, ids, 4);
  zr = fread(msgbuf+8, 1, msglen-8, fp);
  if (zr < msglen - 8) {
    fputs("EOF in GRIB", stderr); r = 1; goto freeret;
  }
  r = scanmsg(msgbuf, msglen);
freeret:
  free(msgbuf);
  /* end ensure malloc-free */
ret:
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
	r = gdecode(fp);
	fprintf(stderr, "%s:%lu: GRIB1 decode %u\n", fnam, lpos, r);
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
