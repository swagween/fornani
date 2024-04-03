#include "Selector.hpp"
#include "Selector.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace gui {

Selector::Selector(sf::Vector2<int> dim) : table_dimensions(dim) {}

void Selector::update() {
	if (table_dimensions.x * table_dimensions.y > 0) {
		current_selection = std::clamp(current_selection, 0, (table_dimensions.x * table_dimensions.y) - 1);
	} else {
		current_selection = 0;
	}
}

void Selector::go_down() {
	current_selection += table_dimensions.x;
	update();
}
void Selector::go_up() {
	current_selection -= table_dimensions.x;
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
