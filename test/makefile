CC=gcc
AR=ar
CFLAGS=-I.
LIBS=-lm -lpthread
LDFLAGS=-lm -lpthread
nocyg=-mwindows

SRC=test.c common.c nlablexer.c nlabparser.c criteria.c nmath.c
	
compile:
	gcc -g -Werror -DDEBUG -o bin/test ../cpp/test.cpp ../cpp/common.cpp ../cpp/nlablexer.cpp ../cpp/nlabparser.cpp ../cpp/criteria.cpp ../cpp/nmath.cpp $(LDFLAGS)