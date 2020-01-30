#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <regex.h>

#include "gribscan.h"

struct cfgout_t {
  time_t rtime;
  unsigned ft;
  int level;
  int param;
};

static unsigned cfgsize = 0;
static struct cfgout_t *cfg = NULL;

static regex_t re_wholeplane;

  int
new_cfgout(unsigned siz)
{
  int r;
  cfgsize = siz;
  cfg = calloc(cfgsize, sizeof(struct cfgout_t));
  if (cfg == NULL) {
    return ENOMEM;
  }
#if 0
  for (i = 0; i < cfgsize; i++) {
    cfg[i].param = 0;
  }
#endif
  r = regcomp(&re_wholeplane, "C([0-9]+)G([0-9]+)P([0-9]+)L([0-9]+)"
    "R([0-9]{4})-([01][0-9])-([0123][0-9])T([012][0-9])Z\\.png",
    REG_EXTENDED | REG_ICASE);
  if (r != 0) return r;
  return 0;
}

  int
parse_cfg(struct cfgout_t *ent, const char *arg)
{
  return 0;
}

  int
store_cfgout(const char *arg)
{
  int i, r;
  for (i = 0; i < cfgsize; i++) {
    if (cfg[i].level == 0) {
      r = parse_cfg(cfg + i, arg);
      if (r != 0) return r;
    }
  }
  return ENOSPC;
}
