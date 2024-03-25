#include "Subconsole.hpp"

namespace gui {

	Subconsole::Subconsole(automa::ServiceProvider& svc) {
	final_dimensions = sf::Vector2<float>{svc.constants.screen_dimensions.x - 4 * pad, svc.constants.screen_dimensions.y / height_factor};
	position = sf::Vector2<float>{origin.x, origin.y - final_dimensions.y};
	text_origin = sf::Vector2<float>{20.0f, 20.0f};
}

} // namespace gui
