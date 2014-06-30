CC=gcc
AR=ar
CFLAGS=-DTARGET_HOST_MS_WINDOWS -I../build/include/ -I/d/Software/opengl/glew-1.10.0/include
LDFLAGS=-lm -lpthread
LIBDIR=-L../build/lib -L/d/Software/opengl/glew-1.10.0/lib
nocyg=-mwindows

x86:
	$(CC) -Wall -Werror -c -o bin/plotter32.exe plotter.cpp $(nocyg) $(CFLAGS) $(LIBDIR) -lnmath32  $(LDFLAGS)
	
m64:
	$(CC) -m64 -Wall -Werror -c -o bin/plotter64.exe plotter.cpp $(nocyg) $(CFLAGS) $(LIBDIR) -lnmath64  $(LDFLAGS)

clean:
	rm ./bin/*.*
