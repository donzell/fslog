AR = ar r
CXX = g++

INCS += -Iinclude

CXXFLAGS=-pipe -g -fpic -Wall -Wextra -Wconversion -Wno-unused-parameter -Wno-old-style-cast -Woverloaded-virtual -Wpointer-arith -Wwrite-strings $(INCS)
SOURCES = $(wildcard src/*.cc)
OBJS=${SOURCES:%.cc=%.o}
LIBRARY_DIST=libfslog.a

all: $(LIBRARY_DIST)
	if [ ! -d output/lib ] ;then mkdir -p output/lib;fi
	rm -rf output/lib/*;	mv $(LIBRARY_DIST) output/lib/
	if [ ! -d output/include ];then mkdir -p output/include;fi
	rm -rf output/include/*; cp include/* output/include/ -r

$(LIBRARY_DIST):$(OBJS)
	$(AR) $@  $^

.PHONY:clean
clean:
	rm -f $(LIBRARY_DIST) $(OBJS)
	rm -rf output
