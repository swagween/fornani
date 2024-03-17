#pragma once
#include <SFML/Graphics.hpp>

namespace style {

class Style {
  public:
	struct {
		sf::Color ui_white{254, 252, 216};
		sf::Color green{81, 186, 155};
		sf::Color dark_orange{210, 71, 0};
	} colors{};
};

} // namespace style