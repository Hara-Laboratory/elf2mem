#if !defined(BYTEORDER_H_READ)
#define BYTEORDER_H_READ
#include <vector>
class byteorder {
	private:
		std::vector<int> order_;
	public:
		byteorder(std::vector<int> order) : order_(order) {};
		int index(int) const;

		static byteorder bigendian(int n);
		static byteorder littleendian(int n);
};
#endif
