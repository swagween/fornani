
#include "editor/tool/Tool.hpp"

#include <algorithm>

namespace pi {

void Marquee::update(Canvas& canvas) {
	Tool::update(canvas);
	if (just_clicked) {
		just_clicked = false;
		if (canvas.hovered()) {
			clicked_position = position;
			activate();
			selection = SelectBox(scaled_clicked_position(), {}, selection_type);
			mode = SelectMode::select;
		}
	}
	if (just_released) {}
	sf::Vector2<uint32_t> dim = {static_cast<uint32_t>(canvas.get_real_dimensions().x), static_cast<uint32_t>(canvas.get_real_dimensions().y)};
	if (!in_bounds(dim) || !active) { return; }
	if (!selection) { return; }
	if (canvas.get_selection_type() != selection_type) { return; }

	sf::Vector2<uint32_t> adjustment{};
	auto x = scaled_clicked_position().x > scaled_position().x ? scaled_position().x : scaled_position_ceiling().x;
	auto y = scaled_clicked_position().y > scaled_position().y ? scaled_position().y : scaled_position_ceiling().y;
	auto boundary_position = sf::Vector2<int>{static_cast<int>(x), static_cast<int>(y)};
	auto real_diff = boundary_position - sf::Vector2<int>{scaled_clicked_position()};

	auto diff = sf::Vector2u{static_cast<uint32_t>(abs(real_diff.x)), static_cast<uint32_t>(abs(real_diff.y))};
	// positive selection
	if (scaled_position().x >= scaled_clicked_position().x) {
		adjustment.x = diff.x;
		selection.value().position.x = scaled_clicked_position().x;
	}
	if (scaled_position().y >= scaled_clicked_position().y) {
		adjustment.y = diff.y;
		selection.value().position.y = scaled_clicked_position().y;
	}
	// negative selection
	if (scaled_position().x < scaled_clicked_position().x) {
		adjustment.x = diff.x + 1;
		selection.value().position.x = scaled_position().x;
	}
	if (scaled_position().y < scaled_clicked_position().y) {
		adjustment.y = diff.y + 1;
		selection.value().position.y = scaled_position().y;
	}
	selection.value().adjust(adjustment);

	selection.value().dimensions.x = std::clamp(selection.value().dimensions.x, 0u, canvas.dimensions.x - selection.value().position.x);
	selection.value().dimensions.y = std::clamp(selection.value().dimensions.y, 0u, canvas.dimensions.y - selection.value().position.y);

	has_palette_selection = selection.value().get_type() == SelectionType::palette;
}

void Marquee::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {}

void Marquee::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset) {
	sf::RectangleShape box{};
	switch (mode) {
	case SelectMode::clipboard:
		[[fallthrough]];
	case SelectMode::select:
		if (!selection) { break; }
		if (canvas.get_selection_type() != selection.value().get_type()) { break; } // don't render if the types don't match
		std::uint8_t red = selection.value().get_type() == SelectionType::canvas ? 100 : 255;
		box.setOutlineColor(sf::Color{red, 255, 160, 180});
		box.setFillColor(sf::Color{red, 100, 180, 40});
		box.setOutlineThickness(-2);
		box.setSize({selection.value().dimensions.x * canvas.f_cell_size(), selection.value().dimensions.y * canvas.f_cell_size()});
		box.setPosition(selection.value().f_position() * canvas.f_cell_size() + offset);
		win.draw(box);
		break;
	}
}

void Marquee::store_tile(int index) {}

void Marquee::clear() {
	if (!selection) { return; }
	if (!selection.value().empty()) {
	}
}

} // namespace pi
