OBJS=gribscan.o cfgout.o
LIBS= -lm
LFLAGS= -g
CFLAGS= -g --pedantic -Wall
CC=cc

.SUFFIXES:
.SUFFIXES: .c .o

grib1scan: $(OBJS)
	$(CC) $(LFLAGS) -o grib1scan $(OBJS) $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) $<

gribscan.o: gribscan.h
cfgout.o: gribscan.h

thinsample.json: thinsamplegen.rb
	ruby thinsamplegen.rb > thinsample.json
