#if !defined(MEMCHUNK_H_READ)
#define MEMCHUNK_H_READ
#include<cstddef>
#include<vector>
#include<map>
#include<tuple>

namespace memory {
class Chunk {
	private:
		bool zero;
		size_t length;
		std::vector<char> mem;
	public:
		Chunk(size_t);
		Chunk(std::vector<char> mem);
		std::vector<char> &getContaint();
		bool isZero();
};
class Memory {
	private:
		size_t entry_;
		std::map<size_t, Chunk> chunks;
	public:
		void entry(size_t x) { entry_ = x; };
		size_t entry() { return entry_; };
		bool addChunk(size_t start_addr, Chunk chunk);
		std::pair<size_t, Chunk> popChunk(void);
		size_t firstAddress(void);
		bool empty(void);
};
}
#endif
