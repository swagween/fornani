
#pragma once
#include "Console.hpp"
#include "Selector.hpp"

namespace gui {

class Subconsole : public Console {
  public:
	Subconsole();
	std::vector<std::string_view> options{};
	Selector selector{};
};

} // namespace gui
