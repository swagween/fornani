#include "ActionControlIcon.hpp"
#include "../service/ServiceProvider.hpp"
#include "../utils/Math.hpp"
#include <algorithm>

namespace gui {

ActionControlIcon::ActionControlIcon(automa::ServiceProvider& svc, IconType type, sf::Vector2<int> lookup) {
	sprite.setTexture(svc.assets.t_controller_button_icons);
	lookup.y += static_cast<int>(type) * 3; // controllers first (3 rows), and keyboard last (8 rows)
	auto atlas_key = sf::Vector2<int>{lookup.x * dimensions.x, lookup.y * dimensions.y};
	sprite.setTextureRect(sf::IntRect{atlas_key, dimensions});
}

void ActionControlIcon::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	sprite.setPosition(position - cam);
	win.draw(sprite);
}

void ActionControlIcon::set_position(sf::Vector2<float> pos) { position = pos; }

} // namespace gui
