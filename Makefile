OBJS=gribscan.o cfgout.o
LIBS= -lm
LFLAGS= -g
CFLAGS= -g --pedantic -std=gnu99 -Wall -Werror
CC=cc

.SUFFIXES:
.SUFFIXES: .c .o

grib1scan: $(OBJS)
	$(CC) $(LFLAGS) -o grib1scan $(OBJS) $(LIBS)

xweight: xweight.o pngout.o
	$(CC) $(LFLAGS) -o xweight xweight.o pngout.o -lm -lpng

.c.o:
	$(CC) -c $(CFLAGS) $<

gribscan.o: gribscan.h
cfgout.o: gribscan.h

clean:
	rm -f $(OBJS)

thinsample.json: thinsamplegen.rb
	ruby thinsamplegen.rb > thinsample.json

tags: cfgout.c gribscan.c pngout.c xweight.c gribscan.h pngout.h
	ctags cfgout.c gribscan.c pngout.c xweight.c gribscan.h pngout.h
