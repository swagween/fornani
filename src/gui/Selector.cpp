#include "Selector.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace gui {

Selector::Selector(automa::ServiceProvider& svc, sf::Vector2<int> dim) : table_dimensions(dim), current_selection(dim.x * dim.y), m_services(&svc) {
	sprite.setTexture(svc.assets.t_selector);
	sprite.setTextureRect(sf::IntRect({0, 0}, {52, 52}));
	sprite.setOrigin(10, 10);
}

void Selector::update() {
	sprite.setPosition(position);
	section = static_cast<InventorySection>(sections.vertical.get());
}

void Selector::render(sf::RenderWindow& win) const { win.draw(sprite); }

void Selector::switch_sections(sf::Vector2<int> way) {
	sections.vertical.modulate(way.y);
	flags.set(SelectorFlags::switched);
}

void Selector::go_down(bool has_arsenal) {
	if (current_selection.get() + table_dimensions.x >= current_selection.get_order()) {
		if (has_arsenal) { switch_sections({0, 1}); }
	} else {
		current_selection.modulate(table_dimensions.x);
	}
	m_services->soundboard.flags.console.set(audio::Console::shift);
}

void Selector::go_up(bool has_arsenal) {
	if (current_selection.get() - table_dimensions.x < 0) {
		if (has_arsenal) { switch_sections({0, -1}); }
	} else {
		current_selection.modulate(-table_dimensions.x);
	}
	m_services->soundboard.flags.console.set(audio::Console::shift);
}

void Selector::go_left() {
	current_selection.modulate(-1);
	m_services->soundboard.flags.console.set(audio::Console::shift);
}

void Selector::go_right() {
	current_selection.modulate(1);
	m_services->soundboard.flags.console.set(audio::Console::shift);
}

void Selector::set_size(int size) { current_selection = util::Circuit(size); }

void Selector::set_dimensions(sf::Vector2<int> dim) { table_dimensions = dim; }

} // namespace gui
