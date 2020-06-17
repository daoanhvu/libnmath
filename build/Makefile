INCDIR = ../include
OUTDIR = ../output
CC = gcc
CPP = g++
# CFLAGS = -I$(INCDIR) -fPIC -std=c++11 -static-libstdc++ -mcmodel=medium
# CFLAGS = -I$(INCDIR) -std=c++11 -fPIC -mcmodel=medium
CFLAGS = -I$(INCDIR)
LIBS = -lm -lpthread

OBJS = $(OUTDIR)/internal_common.o $(OUTDIR)/utf8.o $(OUTDIR)/nlablexer.o $(OUTDIR)/logging.o $(OUTDIR)/StackUtil.o

all: buildtest

$(OUTDIR)/%.o:  ../src/utf8.cpp ../src/nlablexer.cpp ../src/StackUtil.cpp
	$(CPP) -Wall -g -c -o $(OUTDIR)/internal_common.o ../src/internal_common.cpp $(CFLAGS)
	$(CPP) -Wall -g -c -o $(OUTDIR)/utf8.o ../src/utf8.cpp $(CFLAGS)
	$(CPP) -Wall -g -c -o $(OUTDIR)/nlablexer.o ../src/nlablexer.cpp $(CFLAGS)
	$(CPP) -Wall -g -c -o $(OUTDIR)/logging.o ../src/logging.cpp $(CFLAGS)
	$(CPP) -Wall -g -c -o $(OUTDIR)/StackUtil.o ../src/StackUtil.cpp $(CFLAGS)

buildtest: $(OBJS)
	$(CPP) -g -o ../output/$@ ../test/test.cpp $^ $(CFLAGS) $(LIBS)
	$(CPP) -g -o ../output/testglm ../test/testglm.cpp -I../glm $(CFLAGS) $(LIBS)
	$(CPP) -g -o ../output/testImageData ../test/testImageData.cpp -I../glm $(CFLAGS) $(LIBS)
	$(CPP) -g -o ../output/testNLabLexer ../test/testNLabLexer.cpp $^ $(CFLAGS) $(LIBS)
	$(CPP) -g -o ../output/testFunction ../test/testFunction.cpp $^ $(CFLAGS) $(LIBS)

.PHONY: clean
	
clean:
	rm -f $(OUTDIR)/*.o
	rm -f $(OUTDIR)/test
