#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

namespace style {

class Style {
  public:
	struct {
		sf::Color white{235, 232, 249};
		sf::Color ui_white{254, 252, 216};
		sf::Color green{81, 186, 155};
		sf::Color bright_orange{255, 131, 28};
		sf::Color dark_orange{210, 71, 0};
		sf::Color periwinkle{159, 138, 247};
		sf::Color fucshia{215, 53, 180};
		sf::Color goldenrod{247, 199, 74};
	} colors{};

	std::unordered_map<std::string_view, sf::Color> spray_colors{{"clover", colors.green},			{"bryn's gun", colors.periwinkle}, {"plasmer", colors.fucshia},
																 {"skycorps ar", colors.goldenrod}, {"tomahawk", colors.white},		   {"grappling hook", colors.periwinkle}
	};
};

} // namespace style