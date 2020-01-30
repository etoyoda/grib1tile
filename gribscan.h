/* === gribscan.h === */

typedef enum gribscan_err_t {
  GSE_OKAY = 0,
  GSE_JUSTWARN,
  ERR_NOINPUT,
  ERR_OUTPAT,
  ERR_REGEX,
  ERR_TOOMANYCFG,
  ERR_OVERRUN,
  ERR_NOMEM,
  ERR_IO,
  ERR_BADGRIB
} gribscan_err_t;

/* --- cfgout.c --- */

enum gribscan_err_t new_cfgout(unsigned size);
enum gribscan_err_t store_cfgout(const char *arg);
