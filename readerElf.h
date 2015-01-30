#if !defined(READERELF_H_READ)
#define READERELF_H_READ
#include <stdio.h>
#include "memchunk.h"

extern unsigned int read_elf(memory::Memory &mem, FILE *fp);
#endif
