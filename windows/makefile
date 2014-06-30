CC=gcc
AR=ar
CFLAGS=-I.
LIBS=-lm -lpthread
LDFLAGS=-lm -lpthread
nocyg=-mwindows

SRC=common.c nlablexer.c nlabparser2.c nmath.c

nmath:
	$(CC) -Wall -Werror -c -o bin/common.o common.c -lm
	$(CC) -Wall -Werror -c -o bin/nlablexer.o nlablexer.c
	$(CC) -Wall -Werror -c -o bin/nlabparser.o nlabparser2.c -lm
	$(CC) -Wall -Werror -c -o bin/nmath.o nmath.c $(LDFLAGS)
	$(AR) -rc -o ../build/lib/libnmath32.a bin/common.o bin/nlablexer.o bin/nlabparser.o bin/nmath.o
	
m64:
	$(CC) -m64 -Wall -Werror $(nocyg) -static -c $(SRC)
	
test:
	gcc -Wall -Werror -o bin/test test.c common.c nlablexer.c nlabparser2.c nmath.c $(LDFLAGS)
	
test2:
	gcc -Wall -Werror -o bin/test test2.c libfunct.a -lm -lpthread
	
install:
	#rm ../build/include/*.*
	cp ./nlablexer.h 	../build/include/
	cp ./common.h 		../build/include/
	cp ./nmath.h 		../build/include/
	cp ./nlabparser2.h 	../build/include/

clean:
	rm ./bin/*.*
	rm ../build/lib/*.*
	rm ../build/include/*.*

shared:
#	Build shared library
	gcc -c -Wall -Werror -fpic ../nmbase.c funct.c -lm -lpthread
	gcc -shared -o libnmath.so funct.o 
	gcc -L./ -Wall -o test test.c -lnmath -lm -lpthread
