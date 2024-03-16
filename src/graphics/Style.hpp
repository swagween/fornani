#pragma once
#include <SFML/Graphics.hpp>

namespace style {

class Style {
  public:
	struct {
		sf::Color ui_white{254, 252, 216};
	} colors{};
};

} // namespace style