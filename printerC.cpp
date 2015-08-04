#include <boost/algorithm/string.hpp>
#include <stdio.h>
#include <limits>
#include <iomanip>
#include <iostream>
#include "printerC.h"

#define ELEMENT_PER_LINE 16

using namespace memory;

const char* char_type(int width) {
	switch (width) {
		case 8:
			return "uint8_t";
		case 16:
			return "uint16_t";
		case 24:
			return "uint24_t";
		case 32:
			return "uint32_t";
		case 64:
			return "uint64_t";
		default:
			std::cerr << "unsupported bit-width" << std::endl;
			exit(EXIT_FAILURE);
			break;
	}
}

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

void print_mem_elem(std::vector<std::ostream *> &outs, size_t pos, int width, uint64_t v) {
	auto outn = pos / ceilDiv(width, 8) % outs.size();
	auto nelem = pos / ceilDiv(width, 8) / outs.size();
	const char *header = (nelem % ELEMENT_PER_LINE == 0) ? "\t" : "";
	const char *footer = (nelem % ELEMENT_PER_LINE == (ELEMENT_PER_LINE - 1)) ? ",\n" : ", ";
	// *outs[outn] << "/* " << std::hex << std::showbase << pos << "*/";
	*outs[outn] << header << "0x" << std::hex << std::noshowbase << std::setfill('0') << std::setw(ceilDiv(width, 4)) << v << footer;
}

void print_symbols(std::ostream &out, const std::map<std::string, size_t> &symbols) {
	for (auto && p : symbols) {
		auto & name = p.first;
		auto & addr = p.second;
		out << "#define " << boost::to_upper_copy(name) << "_ROUTINE " << addr << std::endl;
	}
}

void print_mem_header(const char *name, std::vector<std::ostream *> &outs, int width, size_t pos) {
	if (outs.size() > 0) {
		*outs[0] << "const size_t " << name << "_address_begin = " << std::hex << std::showbase << pos << ";" << std::endl;
	}
	if (outs.size() == 1) {
		*outs[0] << char_type(width) << " " << name << "[] = {" << std::endl;
	} else {
		for (size_t i = 0; i < outs.size(); ++i) {
			*outs[i] << char_type(width) << " " << name << std::dec << i << "[] = {" << std::endl;
		}
	}
}

void print_mem_footer(const char *name, std::vector<std::ostream *> &outs, int width, size_t start, size_t pos) {
	for (size_t i = 0; i < outs.size(); ++i) {
		*outs[i] << "};" << std::endl;
	}
	if (outs.size() > 0) {
		*outs[outs.size() - 1] << "const size_t " << name << "_address_end = " << std::hex << std::showbase << pos << ";" << std::endl;
		*outs[outs.size() - 1] << "const size_t " << name << "_index_end = " << std::hex << std::showbase << ceilDiv(pos - start, width / 8 * outs.size()) << ";" << std::endl;
	}
}

void printerC::print_mem(std::vector<std::ostream *> &outs, Memory mem) {
	size_t start = start_address_set_ ? start_address_ : mem.firstAddress();
	size_t pos = start;

	print_mem_header(name_, outs, width_, pos);
	while (!mem.empty()) {
		auto t = mem.popChunk();
		size_t addr = t.first;
		auto &chunk = t.second;
		auto ch = t.second.getContaint();
		if (!chunk.name().empty()) {
		    std::cerr << "Writing chunk '" << t.second.name() << "'." << std::endl;
		} else {
		    std::cerr << "Writing anonymous chunk." << std::endl;
		}
		while (pos < addr && (!end_address_set_ || pos < end_address_)) {
			print_mem_elem(outs, pos, width_, 0);
			pos += width_ / 8;
		}
		if (end_address_set_ && pos >= end_address_) {
			std::cerr << "end address (" << end_address_ << ") is small" << " (current position: " << pos << ")" << std::endl;
			exit(EXIT_FAILURE);
		} else if (pos != addr) {
			std::cerr << "address is not aligned" << " (current position: " << pos << ", expected: " << addr << ")" << std::endl;
			exit(EXIT_FAILURE);
		}
		while (pos < addr + ch.size() && (!end_address_set_ || pos < end_address_)) {
			uint64_t v = 0;
			for (int i = 0; i < width_ / 8; i++) {
				v <<= 8;
				v |= ch [pos - addr + order_.index(i)];
			}
			print_mem_elem(outs, pos, width_, v);

			pos += width_ / 8;
		}
	}
	if (end_address_set_) {
		while (pos < end_address_) {
			print_mem_elem(outs, pos, width_, 0);

			pos += width_ / 8;
		}
	}
	print_mem_footer(name_, outs, width_, start, pos);
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

	for (int i = 0; i < split_; ++i) {
		ost << outstrstreams[i]->str();
	}
}
