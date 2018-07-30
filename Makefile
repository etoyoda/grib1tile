OBJS=gribscan.o
LIBS=
CFLAGS=

.SUFFIXES:
.SUFFIXES: .c .o

grib1scan:
	$(CC) -o grib1scan $(OBJS) $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) $<

