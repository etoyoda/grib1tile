/* === gribscan.h === */

enum gribscan_errcode_t {
  ERR_NOINPUT = 2,
  ERR_OUTPAT,
  ERR_REGEX,
  ERR_TOOMANYCFG
};

/* --- cfgout.c --- */

int new_cfgout(unsigned size);
int store_cfgout(const char *arg);
