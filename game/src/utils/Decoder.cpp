#include "fornani/utils/Decoder.hpp"
#include <iostream>
#include <sstream>

namespace util {

std::vector<int> Decoder::decode(std::string const& input, char const& delimiter) {
	std::vector<int> ret{};
	int last_pos = 0;
	for (int i = 0; i < input.size(); ++i) {
		if (input[i] == delimiter) {
			ret.push_back(stoi(input.substr(last_pos, i - last_pos)));
			last_pos = i + 1;
		}
	}
	return ret;
}

} // namespace util
