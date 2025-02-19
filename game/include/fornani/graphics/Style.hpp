#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

namespace fornani::style {

class Style {
  public:
	struct {
		sf::Color nani_white{234, 227, 255};
		sf::Color white{235, 232, 249};
		sf::Color ui_white{254, 252, 216};
		sf::Color green{81, 186, 155};
		sf::Color bright_orange{255, 131, 28};
		sf::Color dark_orange{210, 71, 0};
		sf::Color periwinkle{159, 138, 247};
		sf::Color fucshia{215, 53, 180};
		sf::Color goldenrod{247, 199, 74};
		sf::Color dark_goldenrod{208, 119, 0};
		sf::Color red{255, 7, 100};
		sf::Color blue{110, 98, 173};
		sf::Color navy_blue{25, 35, 65};
		sf::Color dark_fucshia{148, 40, 84};
		sf::Color orange{226, 93, 11};
		sf::Color dark_grey{70, 67, 97};
		sf::Color black{14, 6, 11};
		sf::Color night{19, 18, 16};
		sf::Color ui_black{12, 12, 20};
		sf::Color mythic_green{175, 227, 125};
		sf::Color console_blue{24, 34, 67};
		sf::Color beige{217, 214, 160};
		sf::Color treasure_blue{15, 104, 248};
		sf::Color bright_purple{219, 194, 255};
		sf::Color pioneer_red{229, 59, 68};
	} colors{};

	std::unordered_map<std::string_view, sf::Color> spray_colors{{"clover", colors.green},
																 {"bryn's gun", colors.periwinkle},
																 {"plasmer", colors.fucshia},
																 {"skycorps ar", colors.goldenrod},
																 {"tomahawk", colors.white},
																 {"grappling hook", colors.ui_white},
																 {"grenade launcher", colors.goldenrod},
																 {"gnat", colors.mythic_green},
																 {"Wasp", colors.ui_white}};
};

} // namespace fornani::style
