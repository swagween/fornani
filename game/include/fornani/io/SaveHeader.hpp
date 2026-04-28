
#pragma once

#include <cstdint>
#include <vector>

namespace fornani {

static inline std::uint32_t checksum(std::vector<char> const& data) {
	std::uint32_t sum = 0;
	for (unsigned char c : data) sum = (sum * 31) + c;
	return sum;
}

struct SaveHeader {
	std::uint32_t magic = 0x464F524E; // 'FORN'
	std::uint32_t version = 1;
	std::uint32_t data_size = 0;
};

} // namespace fornani
