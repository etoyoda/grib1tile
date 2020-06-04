#define HAS_TIMEGM 1

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

#define NXOUT 288
#define NYOUT 289

struct cfgout_t {
  unsigned ctr;
  unsigned gen;
  unsigned par;
  unsigned ft;
  unsigned lev;
  time_t rt;
  float *acc;
  float *wgt;
};

extern enum gribscan_err_t new_cfgout(unsigned size);
extern enum gribscan_err_t store_cfgout(const char *arg);
extern time_t timegm6(unsigned y, unsigned m, unsigned d,
  unsigned h, unsigned n, unsigned s);
extern struct cfgout_t *
  check_msg(unsigned ctr, unsigned gen, unsigned par, unsigned ft,
  unsigned lev, struct tm *rtime, unsigned igrid);

/* --- gribscan.c --- */

extern const char *showtime(char *buf, size_t size, const struct tm *t);
