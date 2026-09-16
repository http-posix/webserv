// c++ -std=c++17 -Wall -Wextra -Werror buckets.cpp -o buckets && ./buckets

#include <iostream>
#include <unordered_map>

int main() {
	// 1. Big map: fd 0..1023
	std::unordered_map<int, int> map;
	for (int fd = 0; fd < 1024; ++fd)
		map[fd] = fd;

	size_t max_chain = 0, used = 0;
	for (size_t b = 0; b < map.bucket_count(); ++b) {
		size_t len = map.bucket_size(b);
		if (len > 0) ++used;
		if (len > max_chain) max_chain = len;
	}
	std::cout << "elements:     " << map.size() << '\n'
			  << "buckets:      " << map.bucket_count() << '\n'
			  << "used buckets: " << used << '\n'
			  << "max chain:    " << max_chain << "\n\n";

	// 2. Small map
	std::unordered_map<int, int> small;
	small[5] = 5;
	size_t bc = small.bucket_count();
	int a = 5, b = 5 + bc, c = 5 + 2 * bc;
	small[b] = b;
	small[c] = c;
	std::cout << "small map, buckets: " << small.bucket_count() << '\n';
	for (int fd : {a, b, c})
			std::cout << "fd " << fd
				  << " -> hash " << std::hash<int>{}(fd)
				  << " -> bucket " << small.bucket(fd) << '\n';
		

	// 3. Reserve size: same fds that collided in the small map
	std::unordered_map<int, int> small_reserve;
	small_reserve.reserve(1024);
	for (int fd : {5, 18, 31})
		small_reserve[fd] = fd;
	std::cout << "\nsmall_reserve map, buckets: " << small_reserve.bucket_count() << '\n';
	for (int fd : {5, 18, 31})
		std::cout << "fd " << fd
				  << " -> hash " << std::hash<int>{}(fd)
				  << " -> bucket " << small_reserve.bucket(fd) << '\n';

}

/*
Expected output:
elements:     1024
buckets:      1109
used buckets: 1024
max chain:    1

small map, buckets: 13
fd 5 -> hash 5 -> bucket 5
fd 18 -> hash 18 -> bucket 5
fd 31 -> hash 31 -> bucket 5

small_reserve map, buckets: 1031
fd 5 -> hash 5 -> bucket 5
fd 18 -> hash 18 -> bucket 18
fd 31 -> hash 31 -> bucket 31

*/
