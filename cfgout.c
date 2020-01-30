#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <regex.h>

#include "gribscan.h"

struct cfgout_t {
  unsigned ctr;
  unsigned gen;
  unsigned par;
  unsigned ft;
  unsigned lev;
  time_t rt;
};

static unsigned cfgsize = 0;
static struct cfgout_t *cfg = NULL;

static regex_t re_wholeplane;

  enum gribscan_err_t
new_cfgout(unsigned siz)
{
  int r;
  cfgsize = siz;
  /* calloc でゼロクリアする。 par がゼロを未使用の印として使う */
  cfg = calloc(cfgsize, sizeof(struct cfgout_t));
  if (cfg == NULL) {
    return ERR_NOMEM;
  }
  /* ついでに正規表現をコンパイルしておく */
  /*                            1        2        3        4        5 */
  r = regcomp(&re_wholeplane, "C([0-9]+)G([0-9]+)P([0-9]+)F([0-9]+)L([0-9]+)"
  /*  6          7           8             9 */
    "R([0-9]{4})-([01][0-9])-([0123][0-9])T([012][0-9])Z\\.png",
    REG_EXTENDED | REG_ICASE);
  if (r != 0) return ERR_REGEX;
  return GSE_OKAY;
}

  void
regexmsg(int e, const regex_t *re)
{
  char buf[256];
  regerror(e, re, buf, sizeof buf);
  fputs(buf, stderr);
}

  time_t
timegm6(unsigned y, unsigned m, unsigned d, unsigned h, unsigned n, unsigned s)
{
  char *tz;
  time_t result;
  struct tm tmbuf;
  tz = getenv("TZ");
  setenv("TZ", "", 1);
  tzset();
  tmbuf.tm_year = y;
  tmbuf.tm_mon = m;
  tmbuf.tm_mday = d;
  tmbuf.tm_hour = h;
  tmbuf.tm_min = n;
  tmbuf.tm_sec = s;
  tmbuf.tm_isdst = 0;
  result = mktime(&tmbuf);
  if (tz) {
    setenv("TZ", tz, 1);
  } else {
    unsetenv("TZ");
  }
  tzset();
  return result;
}

  enum gribscan_err_t
parse_cfg(struct cfgout_t *ent, const char *arg)
{
  int r;
  unsigned ry, rm, rd, rh;
  const int NMATCH = 10;
  regmatch_t md[NMATCH];
  r = regexec(&re_wholeplane, arg, NMATCH, md, 0);
  if (r == REG_NOMATCH) {
    fprintf(stderr, "bad filename '%s'\n", arg);
    return ERR_OUTPAT;
  } else if (r) {
    regexmsg(r, &re_wholeplane);
    return ERR_REGEX;
  }
  ent->ctr = strtoul(arg + md[1].rm_so, NULL, 10);
  ent->gen = strtoul(arg + md[2].rm_so, NULL, 10);
  ent->par = strtoul(arg + md[3].rm_so, NULL, 10);
  ent->ft = strtoul(arg + md[4].rm_so, NULL, 10);
  ent->lev = strtoul(arg + md[5].rm_so, NULL, 10);
  ry = strtoul(arg + md[6].rm_so, NULL, 10);
  rm = strtoul(arg + md[7].rm_so, NULL, 10);
  rd = strtoul(arg + md[8].rm_so, NULL, 10);
  rh = strtoul(arg + md[9].rm_so, NULL, 10);
  ent->rt = timegm6(ry, rm, rd, rh, 0u, 0u);
  
  return GSE_OKAY;
}

  enum gribscan_err_t
store_cfgout(const char *arg)
{
  int i;
  enum gribscan_err_t r;
  for (i = 0; i < cfgsize; i++) {
    if (cfg[i].par == 0) {
      r = parse_cfg(cfg + i, arg);
      return r;
    }
  }
  return ERR_TOOMANYCFG;
}
