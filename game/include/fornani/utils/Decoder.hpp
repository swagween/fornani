#pragma once

#include <iostream>
#include <string_view>
#include <vector>

namespace fornani::util {

class Decoder {
  public:
	std::vector<int> decode(std::string const& input, char const& delimiter);
};

} // namespace util