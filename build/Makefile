INCDIR = ../include
OUTDIR = ../output/mac
CC = clang
CPP = clang++
CFLAGS = -I$(INCDIR) -std=c++11 -stdlib=libc++
LIBS = -lm -lpthread

OBJS = $(OUTDIR)/internal_common.o $(OUTDIR)/utf8.o $(OUTDIR)/nlablexer.o $(OUTDIR)/logging.o $(OUTDIR)/StackUtil.o

all: test

$(OUTDIR)/%.o:  ../src/utf8.cpp ../src/nlablexer.cpp ../src/StackUtil.cpp
	$(CPP) -g -c -o $(OUTDIR)/internal_common.o ../src/internal_common.cpp $(CFLAGS)
	$(CPP) -g -c -o $(OUTDIR)/utf8.o ../src/utf8.cpp $(CFLAGS)
	$(CPP) -g -c -o $(OUTDIR)/nlablexer.o ../src/nlablexer.cpp $(CFLAGS)
	$(CPP) -g -c -o $(OUTDIR)/logging.o ../src/logging.cpp $(CFLAGS)
	$(CPP) -g -c -o $(OUTDIR)/StackUtil.o ../src/StackUtil.cpp $(CFLAGS)

test: $(OBJS)
	$(CPP) -g -o ../output/mac/$@ ../test/test.cpp $^ $(CFLAGS) $(LIBS)
	$(CPP) -g -o ../output/mac/testglm ../test/testglm.cpp -I../glm $(CFLAGS) $(LIBS)
	$(CPP) -g -o ../output/mac/testImageData ../test/testImageData.cpp -I../glm $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OUTDIR)/*.o
	rm -f $(OUTDIR)/test