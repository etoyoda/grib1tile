#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "gribscan.h"

struct cfgout_t {
  time_t rtime;
  unsigned ft;
  int level;
  int elem;
};

static struct cfgout_t *cfg = NULL;

  int
new_cfgout(unsigned siz)
{
  cfg = calloc(siz, sizeof(struct cfgout_t));
  if (cfg == NULL) {
    return ENOMEM;
  }
  return 0;
}

  int
store_cfgout(const char *arg)
{
  return 0;
}
