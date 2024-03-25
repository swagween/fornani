#include "InventoryWindow.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

InventoryWindow::InventoryWindow(automa::ServiceProvider& svc) : Console::Console(svc) {
	final_dimensions = sf::Vector2<float>{svc.constants.screen_dimensions.x - 2 * pad, svc.constants.screen_dimensions.y - 2 * pad};
	current_dimensions.x = final_dimensions.x;
	position = sf::Vector2<float>{origin.x, origin.y - final_dimensions.y};
}

} // namespace gui
