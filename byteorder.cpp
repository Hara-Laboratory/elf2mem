#include <stdexcept>
#include <vector>
#include "byteorder.h"

int byteorder::index(int n) const {
	return order_[n];
}

int byteorder::size(void) const {
	return order_.size();
}

byteorder byteorder::bigendian(int n) {
	std::vector<int> v(n);
	for (int i = 0; i < n; ++i) {
		v[i] = i;
	}
	return byteorder(v);
}

byteorder byteorder::littleendian(int n) {
	std::vector<int> v(n);
	for (int i = 0; i < n; ++i) {
		v[i] = n - i - 1;
	}
	return byteorder(v);
}

byteorder byteorder::from_string(std::string type, int n) {
	if (!type.compare("little-endian")) {
		return littleendian(n);
	} else if (!type.compare("big-endian")) {
		return bigendian(n);
	} else {
		throw std::invalid_argument("Unknown byteorder string.");
	}
}
