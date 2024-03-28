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
		sf::Color red{236, 63, 95};
		sf::Color blue{110, 98, 173};
		sf::Color navy_blue{25, 35, 65};
		sf::Color dark_fucshia{148, 40, 84};
		sf::Color orange{226, 93, 11};
		sf::Color dark_grey{55, 49, 64};
		sf::Color black{14, 6, 11};
		sf::Color night{19, 18, 16};
		sf::Color ui_black{12, 12, 20};
	} colors{};

	std::unordered_map<std::string_view, sf::Color> spray_colors{{"clover", colors.green},			{"bryn's gun", colors.periwinkle}, {"plasmer", colors.fucshia},
																 {"skycorps ar", colors.goldenrod}, {"tomahawk", colors.white},		   {"grappling hook", colors.ui_white}
	};
};

} // namespace style