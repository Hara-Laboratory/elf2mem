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

PKGNAME=elf2mem
PKGVER=0.1
 
# http://stackoverflow.com/questions/10858261/abort-makefile-if-variable-not-set
# Check that given variables are set and all have non-empty values,
# die with an error otherwise.
#
# Params:
#   1. Variable name(s) to test.
#   2. (optional) Error message to print.
check_defined = \
    $(foreach 1,$1,$(__check_defined))
__check_defined = \
    $(if $(value $1),, \
      $(error Undefined $1$(if $(value 2), ($(strip $2)))))

.PHONY: all clean install-porg

all: $(TARGET) $(OBJECTS)

clean:
	rm -rf $(TARGET) $(OBJECTS) *.d

install-porg: $(TARGET)
	[ -d "$(BIN_DIR)" ] || mkdir -p "$(BIN_DIR)"
	$(call check_defined, BIN_DIR, where to put binary artifacts)
	porg -lp $(PKGNAME)-$(PKGVER) cp "$(TARGET)" "$(BIN_DIR)/"


$(TARGET): $(OBJECTS)

-include *.d
