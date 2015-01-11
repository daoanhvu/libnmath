CC=g++
AR=ar
CFLAGS=-I../cpp2/ -I../gm/
LIBS=-lm -lpthread
LDFLAGS=-lm -lpthread
SRC=../cpp2/common.cpp \
	../cpp2/nmath_pool.cpp \
	../cpp2/nfunction.cpp \
	../cpp2/nlablexer.cpp \
	../cpp2/nlabparser.cpp \
	../cpp2/criteria.cpp \
	../cpp2/SimpleCriteria.cpp \
	../cpp2/compositecriteria.cpp \
	../cpp2/StringUtil.cpp \
	../cpp2/StackUtil.cpp \
	./test.cpp
	
maketest: $(SRC)
	$(CC) -D_PCDEBUG -I"../cpp2" -I"../gm" $(SRC) $(LIBS)
#	$(CC) -D_DEBUG -D_PCDEBUG -o test $(CFLASG) $(LIBS) 
