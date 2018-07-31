OBJS=gribscan.o
LIBS= -lm
LFLAGS= -g
CFLAGS= -g --pedantic
CC=cc

.SUFFIXES:
.SUFFIXES: .c .o

grib1scan: $(OBJS)
	$(CC) $(LFLAGS) -o grib1scan $(OBJS) $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) $<

