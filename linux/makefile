CC=gcc
AR=ar
CFLAGS=-I.
LIBS=-lm -lpthread
LDFLAGS=-lm -lpthread
nocyg=-mwindows

SRC=test.c common.c nlablexer.c nlabparser.c criteria.c nmath.c

nmath:
	$(CC) -Wall -Werror -c -o bin/common.o common.c -lm
	$(CC) -Wall -Werror -c -o bin/nlablexer.o nlablexer.c
	$(CC) -Wall -Werror -c -o bin/nlabparser.o nlabparser2.c -lm
	$(CC) -Wall -Werror -c -o bin/nmath.o nmath.c $(LDFLAGS)
	$(AR) -rc -o ../build/lib/libnmath32.a bin/common.o bin/nlablexer.o bin/nlabparser.o bin/nmath.o
	
m64:
	$(CC) -m64 -Wall -Werror -c -o bin/common64.o common.c -lm
	$(CC) -m64 -Wall -Werror -c -o bin/nlablexer64.o nlablexer.c
	$(CC) -m64 -Wall -Werror -c -o bin/nlabparser64.o nlabparser2.c -lm
	$(CC) -m64 -Wall -Werror -c -o bin/nmath64.o nmath.c $(LDFLAGS)
	$(AR) -rc -o ../build/lib/libnmath64.a bin/common64.o bin/nlablexer64.o bin/nlabparser64.o bin/nmath64.o
	
test:
	gcc -g -Wall -Werror -DDEBUG -o bin/test test.c common.c nlablexer.c nlabparser.c criteria.c nmath.c $(LDFLAGS)
	
android:
	gcc -Wall -Werror -D_TARGET_HOST_ANDROID -o bin/testa $(SRC) $(LDFLAGS)
	
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
