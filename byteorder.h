#if !defined(BYTEORDER_H_READ)
#define BYTEORDER_H_READ
#include <vector>
#include <string>
struct byteorder {
	private:
		std::vector<int> order_;
	public:
		byteorder(std::vector<int> const& order) : order_(order) {};
		byteorder() = default;
		int index(int) const;
		int size(void) const;

		static byteorder bigendian(int n);
		static byteorder littleendian(int n);
		static byteorder from_string(std::string type, int n);
};
#endif
