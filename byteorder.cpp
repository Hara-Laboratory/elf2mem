#include <vector>
#include "byteorder.h"

int byteorder::index(int n) {
	return order_[n];
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
