CC=g++
AR=ar
CFLAGS=-I../cpp2/ -I../gm/
LIBS=-lm -lpthread
LDFLAGS=-lm -lpthread
nocyg=-mwindows

SRC=../cpp2/common.cpp \
	../cpp2/nmath_pool.cpp \
	../cpp2/nlablexer.cpp \
	../cpp2/nlabparser.cpp \
	../cpp2/criteria.cpp \
	../cpp2/SimpleCriteria.cpp \
	../cpp2/compositecriteria.cpp \
	../cpp2/StringUtil.cpp \
	../cpp2/StackUtil.cpp
	
compile:
	$(CC) -g -Werror -DDEBUG -o bin/test test.cpp $(SRC) $(CFLASG) $(LDFLAGS)
