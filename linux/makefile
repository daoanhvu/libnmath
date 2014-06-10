nmathmake:
	gcc -Wall -Werror -c -o bin/common.o common.c -lm
	gcc -Wall -Werror -c -o bin/nlablexer.o nlablexer.c
	gcc -Wall -Werror -c -o bin/nlabparser.o nlabparser.c -lm
	gcc -Wall -Werror -c -o bin/nmath.o nmath.c -lm -lpthread
	ar -rc -o bin/libnmath.a bin/common.o bin/nlablexer.o bin/nlabparser.o bin/nmath.o
test:
	gcc -Wall -Werror -o bin/test test.c common.c nlablexer.c nlabparser.c nmath.c -lm -lpthread
	
test2:
	gcc -Wall -Werror -o bin/test test2.c libfunct.a -lm -lpthread
	
install:
	mkdir /home/vudao/workspace1/mylib/nmath/include/
	cp ./libfunct.a /home/vudao/workspace1/mylib/nmath/lib/	
	cp ./derivative.h /home/vudao/workspace1/mylib/nmath/include/
	cp ./funct.h /home/vudao/workspace1/mylib/nmath/include/
	cp ./nmbase.h /home/vudao/workspace1/mylib/nmath/include/

clean:
	rm test funct.o nmbase.o libfunct.a

shared:
#	Build shared library
	gcc -c -Wall -Werror -fpic ../nmbase.c funct.c -lm -lpthread
	gcc -shared -o libnmath.so funct.o 
	gcc -L./ -Wall -o test test.c -lnmath -lm -lpthread
