#include "Selector.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace gui {

Selector::Selector(automa::ServiceProvider& svc, sf::Vector2<int> dim) : table_dimensions(dim) {
	sprite.setTexture(svc.assets.t_selector);
	sprite.setTextureRect(sf::IntRect({0, 0}, {52, 52}));
	sprite.setOrigin(10, 10);
}

void Selector::update() {
	if (table_dimensions.x * table_dimensions.y > 0) {
		current_selection = std::clamp(current_selection, 0, (table_dimensions.x * table_dimensions.y) - 1);
	} else {
		current_selection = 0;
	}
	sprite.setPosition(position);
}

void Selector::render(sf::RenderWindow& win) const { win.draw(sprite); }

void Selector::go_down() {
	if (current_selection < table_dimensions.x * table_dimensions.y - table_dimensions.x - 1) { current_selection += table_dimensions.x; }
	update();
}
void Selector::go_up() {
	if (current_selection >= table_dimensions.x) { current_selection -= table_dimensions.x; }
	update();
}
void Selector::go_left() {
	--current_selection;
	update();
}
void Selector::go_right() {
	++current_selection;
	update();
}

void Selector::set_dimensions(sf::Vector2<int> dim) { table_dimensions = dim; }

} // namespace gui
