#include "fornani/gui/Selector.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include <ccmath/ext/clamp.hpp>

namespace fornani::gui {

Selector::Selector(automa::ServiceProvider& svc, sf::Vector2<int> dim) : table_dimensions(dim), current_selection(dim.x * dim.y), m_services(&svc), sprite{svc.assets.t_selector} {
	sprite.setTextureRect(sf::IntRect({0, 0}, {52, 52}));
	sprite.setOrigin({10, 10});
}

void Selector::update() {
	sprite.setPosition(position);
	current_selection.set(ccm::ext::clamp(current_selection.get(), 0, current_selection.get_order() - 1));
}

void Selector::render(sf::RenderWindow& win) const { win.draw(sprite); }

void Selector::switch_sections(sf::Vector2<int> way) {
	sections.vertical.modulate(way.y);
	section = static_cast<InventorySection>(sections.vertical.get());
	flags.set(SelectorFlags::switched);
	if (way.y == -1) { flags.set(SelectorFlags::went_up); }
	table_dimensions.x == 0 ? current_selection.set(0) : current_selection.set(current_selection.get() % table_dimensions.x);
}

void Selector::go_down(bool has_arsenal) {
	if (current_selection.get() + table_dimensions.x >= current_selection.get_order()) {
		if (has_arsenal && last_row()) {
			switch_sections({0, 1});
		} else {
			current_selection.set(current_selection.get_order() - 1);
		}
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

void Selector::set_size(int size) {
	if (size < 1) {
		current_selection.set_order(1);
		return;
	}
	current_selection.set_order(size);
}

void Selector::set_dimensions(sf::Vector2<int> dim) {
	table_dimensions = dim;
	if (table_dimensions.x == 0) {
		current_selection.set(0);
		return;
	}
	if (flags.consume(SelectorFlags::went_up) && table_dimensions.y > 1) {
		if (get_current_selection() < current_selection.get_order() % table_dimensions.x) {
			current_selection.modulate(table_dimensions.x * (table_dimensions.y - 1));
		} else {
			current_selection.set(current_selection.get_order() - 1);
		}
	}
}

bool Selector::last_row() const { return current_selection.get() / table_dimensions.x == table_dimensions.y - 1; }

} // namespace fornani::gui
