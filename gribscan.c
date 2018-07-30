#include <stdio.h>
#include <stdlib.h>

  inline size_t
ui3(const unsigned char *buf)
{
  return (buf[0] << 16) | (buf[1] << 8) | buf[2];
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
#if 0
  memcpy(msgbuf+0, "GRIB", 4);
  memcpy(msgbuf+4, ids, 4);
#endif
  zr = fread(msgbuf+8, 1, msglen-8, fp);
  if (zr < msglen - 8) {
    fputs("EOF in GRIB", stderr); r = 1; goto freeret;
  }
  /* ooo */
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
