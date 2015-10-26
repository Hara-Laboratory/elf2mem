CFLAGS=-g -O2-MMD
#CXXFLAGS=-g -O2 -std=c++11
#CXXFLAGS=-g -Og -std=c++11
#CXXFLAGS=-g -std=c++11 -O3 -Wall -Wno-format-extra-args
#CXXFLAGS=-g -std=c++0x -O3 -Wall -Wno-format-extra-args
CXXFLAGS=-g -std=c++0x -Wall -Wno-format-extra-args -MMD

LDFLAGS=-lelf
SOURCES=byteorder.cpp memchunk.cpp printerC.cpp readerElf.cpp readerSubleqObj.cpp
OBJECTS=$(SOURCES:.cpp=.o)
TARGET=elf2mem
 
all: $(TARGET) $(OBJECTS)

clean:
	rm -rf $(TARGET) $(OBJECTS) *.d

$(TARGET): $(OBJECTS)

-include *.d
