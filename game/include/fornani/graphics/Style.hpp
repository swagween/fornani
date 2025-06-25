#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "fornani/graphics/Colors.hpp"

namespace fornani::style {

class Style {
  public:
	std::unordered_map<std::string_view, sf::Color> spray_colors{{"clover", colors::green},
																 {"bryn's gun", colors::periwinkle},
																 {"plasmer", colors::fucshia},
																 {"skycorps ar", colors::goldenrod},
																 {"tomahawk", colors::white},
																 {"grappling hook", colors::ui_white},
																 {"grenade launcher", colors::goldenrod},
																 {"gnat", colors::mythic_green},
																 {"Wasp", colors::ui_white}};
};

} // namespace fornani::style
