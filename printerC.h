#if !defined(PRINTERC_H_READ)
#define PRINTERC_H_READ
#include <ostream>
#include <sstream>
#include "memchunk.h"

extern void print_mem(const char *name, std::vector<std::ostream *> &outs, size_t start, size_t end, memory::Memory mem) ;
extern void print_mem(const char *name, std::ostream &outs, size_t start, size_t end, memory::Memory mem) ;
#endif
