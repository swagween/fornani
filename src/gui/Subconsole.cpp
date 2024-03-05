#include "Subconsole.hpp"

namespace gui {

	Subconsole::Subconsole() {
	dimensions = sf::Vector2<float>{(float)cam::screen_dimensions.x - 4 * pad, (float)cam::screen_dimensions.y / height_factor};
	position = sf::Vector2<float>{origin.x, origin.y - dimensions.y};
	text_origin = sf::Vector2<float>{20.0f, 20.0f};
}

} // namespace gui
