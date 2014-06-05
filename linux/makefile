nmathmake:
	gcc -Wall -Werror -c -o nmbase.o nmbase.c -lm
	gcc -Wall -Werror -c -o derivative.o derivative.c -lm -lpthread
	gcc -Wall -Werror -c -o funct.o funct.c -lm -lpthread
#	ar rcs libfunct.a nmbase.o derivative.o funct.o
	ar -rc libfunct.a nmbase.o derivative.o funct.o
test:
	gcc -Wall -Werror -o test test.c nmbase.c derivative.c funct.c -lm -lpthread
	
test2:
	gcc -Wall -Werror -o test test2.c libfunct.a -lm -lpthread
	
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
