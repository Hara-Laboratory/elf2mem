#include <stdio.h>
#include <limits>
#include <iomanip>
#include "printerC.h"

#define ELEMENT_PER_LINE 16

using namespace memory;

template <typename T>
static inline T ceilDiv(T dividend, T divisor) {
	if (divisor == 0 ) {
		fprintf(stderr, "zero division\n");
	}
	if (divisor == -1 && dividend == std::numeric_limits<T>::min()) {
		fprintf(stderr, "division overflow\n");
	}

  T roundedTowardsZeroQuotient = dividend / divisor;
  bool dividedEvenly = (dividend % divisor) == 0;
  if (dividedEvenly) 
    return roundedTowardsZeroQuotient;

  // At this point we know that divisor was not zero 
  // (because we would have thrown) and we know that 
  // dividend was not zero (because there would have been no remainder)
  // Therefore both are non-zero.  Either they are of the same sign, 
  // or opposite signs. If they're of opposite sign then we rounded 
  // UP towards zero so we're done. If they're of the same sign then 
  // we rounded DOWN towards zero, so we need to add one.

  bool wasRoundedDown = ((divisor > 0) == (dividend > 0));
  if (wasRoundedDown) 
    return roundedTowardsZeroQuotient + 1;
  else
    return roundedTowardsZeroQuotient;
}

void print_mem_elem(std::vector<std::ostream *> &outs, size_t pos, int v) {
	auto outn = pos % outs.size();
	auto nelem = pos / outs.size();
	const char *header = (nelem % ELEMENT_PER_LINE == 0) ? "\t" : "";
	const char *footer = (nelem % ELEMENT_PER_LINE == (ELEMENT_PER_LINE - 1)) ? ",\n" : ", ";
	// *outs[outn] << header << std::hex << std::showbase << std::setfill('0') << std::setw(2) << v << footer;
	*outs[outn] << header << "0x" << std::hex << std::noshowbase << std::setfill('0') << std::setw(2) << v << footer;
}

void print_mem_header(const char *name, std::vector<std::ostream *> &outs, size_t pos) {
	if (outs.size() > 0) {
		*outs[0] << "const size_t " << name << "_address_begin = " << std::hex << std::showbase << pos << std::endl;
	}
	if (outs.size() == 1) {
		*outs[0] << "unsigned char " << name << "[] = {" << std::endl;
	} else {
		for (size_t i = 0; i < outs.size(); ++i) {
			*outs[i] << "unsigned char " << name << std::dec << i << "[] = {" << std::endl;
		}
	}
}

void print_mem_footer(const char *name, std::vector<std::ostream *> &outs, size_t pos) {
	for (size_t i = 0; i < outs.size(); ++i) {
		*outs[i] << "};" << std::endl;
	}
	if (outs.size() > 0) {
		*outs[outs.size() - 1] << "const size_t " << name << "_address_end = " << std::hex << std::showbase << pos << std::endl;
		*outs[outs.size() - 1] << "const size_t " << name << "_index_end = " << std::hex << std::showbase << ceilDiv(pos, outs.size()) << std::endl;
	}
}

void printerC::print_mem(std::vector<std::ostream *> &outs, Memory mem) {
	size_t pos = start_address_set_ ? start_address_ : mem.firstAddress();

	print_mem_header(name_, outs, pos);
	while (!mem.empty()) {
		auto t = mem.popChunk();
		size_t addr = t.first;
		auto ch = t.second.getContaint();
		for (; pos < addr && (!end_address_set_ || pos < end_address_) ; ++pos) {
			print_mem_elem(outs, pos, 0);
		}
		for (; pos < addr + ch.size() && (!end_address_set_ || pos < end_address_); ++pos) {
			print_mem_elem(outs, pos, static_cast<unsigned char>(ch[pos - addr]));
		}
	}
	if (end_address_set_) {
		for (; pos < end_address_; ++pos) {
			print_mem_elem(outs, pos, 0);
		}
	}
	print_mem_footer(name_, outs, pos);
}

void printerC::print_mem(std::ostream &ost, Memory mem) {
	std::vector<std::ostream *> outstreams;
	std::vector<std::ostringstream *> outstrstreams;
	for (int i = 0; i < split_; ++i) {
		auto ostr = new std::ostringstream();
		outstreams.push_back(ostr);
		outstrstreams.push_back(ostr);
		// outs.push_back(ostr);
	}

	print_mem(outstreams, mem);

	for (auto && ostr : outstrstreams) {
		ost << ostr->str();
	}
}
