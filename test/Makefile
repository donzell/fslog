AR = ar r
CXX = g++

INCS += -I../output/include
LIBS +=	-lgtest -L../output/lib -lfslog -lboost_thread -lpthread

CXXFLAGS=-pipe -g -fpic -Wall -Wextra -Wconversion -Wno-unused-parameter -Wno-old-style-cast -Woverloaded-virtual -Wpointer-arith -Wwrite-strings -rdynamic $(INCS)

SOURCES = $(wildcard *.cc)
OBJS=${SOURCES:%.cc=%.o}
BINS=${SOURCES:%.cc=%}

.PHONY:all
all: $(BINS)
	echo $(BINS)

.PHONY:clean
clean:
	rm -f $(OBJS) $(BINS)

$(BINS):%:%.o
	$(CXX) -o $@ $^ $(LIBS)


