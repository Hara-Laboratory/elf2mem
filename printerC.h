#if !defined(PRINTERC_H_READ)
#define PRINTERC_H_READ
#include <ostream>
#include <sstream>
#include "memchunk.h"
#include "byteorder.h"

class printerC {
	private:
		const size_t start_address_, end_address_;
		const bool start_address_set_, end_address_set_;
		const char *name_;
		const int split_;
		const int width_;
		const byteorder order_;
		const bool print_header_;
		void print_mem(std::vector<std::ostream *> &outs, memory::Memory mem) ;
	public:
		printerC (const char * name, bool start_address_set, size_t start_addr, bool end_address_set, size_t end_addr, int split, int width, byteorder order) :
			start_address_(start_addr), end_address_(end_addr),
			start_address_set_(start_address_set), end_address_set_(end_address_set),
			name_(name),
			split_(split),
			width_(width),
			order_(order)
	{};
		printerC (const char * name, bool start_address_set, size_t start_addr, bool end_address_set, size_t end_addr, int split, int width) :
			start_address_(start_addr), end_address_(end_addr),
			start_address_set_(start_address_set), end_address_set_(end_address_set),
			name_(name),
			split_(split),
			width_(width),
			order_(byteorder::bigendian(width / 8))
	{};
		printerC (const char * name, bool start_address_set, size_t start_addr, bool end_address_set, size_t end_addr, int split) :
			start_address_(start_addr), end_address_(end_addr),
			start_address_set_(start_address_set), end_address_set_(end_address_set),
			name_(name),
			split_(split),
			width_(8),
			order_(byteorder::bigendian(1))
	{};
		// extern void print_mem(const char *name, std::vector<std::ostream *> &outs, size_t start, size_t end, memory::Memory mem) ;
		void print_mem(std::ostream &outs, memory::Memory mem) ;
};
#endif
