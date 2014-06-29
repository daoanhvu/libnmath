CC=gcc
AR=ar
CFLAGS=-I../build/include/
LIBS=-lm -lpthread
LIBDIR=-L../build/lib

x86:
	$(CC) -Wall -Werror -c -o bin/plotter32 plotter.cpp $(CFLAGS) $(LIBDIR) -lnmath32  $(LIBS)
	
m64:
	$(CC) -m64 -Wall -Werror -c -o bin/plotter64 plotter.cpp $(CFLAGS) $(LIBDIR) -lnmath32  $(LIBS)

clean:
	rm ./bin/*.*
