#include <cstddef>
#include <vector>
#include <map>
#include <tuple>
#include "memchunk.h"

namespace memory {
	bool Chunk::isZero() { return zero_; }

	std::vector<unsigned char> &Chunk::getContaint(){ return mem_; }

	Chunk::Chunk(std::vector<unsigned char> mem) : zero_(0), length_(mem.size()), mem_(mem), name_("") {}

	Chunk::Chunk(std::string name, std::vector<unsigned char> mem) : zero_(0), length_(mem.size()), mem_(mem), name_(name) {}

	const std::string &Chunk::name() { return name_; }

	bool Memory::addChunk(size_t start_addr, Chunk chunk) {
		chunks.insert(std::make_pair(start_addr, chunk));
		return true;
	}

	std::pair<size_t, Chunk> Memory::popChunk(void) {
		auto it = chunks.begin();
		auto v = it->second;
		auto k = it->first;
		chunks.erase(k);
		return std::make_pair(k, v);
	}

	size_t Memory::firstAddress(void) {
		auto it = chunks.begin();
		return it->first;
	}

	bool Memory::empty(void) { return chunks.empty(); }
}
