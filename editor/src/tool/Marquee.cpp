
#include "editor/tool/Tool.hpp"

#include <algorithm>

namespace pi {

void Marquee::update(Canvas& canvas) {
	Tool::update(canvas);
	if (just_clicked) {
		clicked_position = position;
		just_clicked = false;
		activate();
		if (canvas.hovered()) {
			selection = SelectBox(scaled_clicked_position(), {}, selection_type);
			clipboard = {};
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
		adjustment.x = diff.x;
		selection.value().position.x = scaled_position().x;
	}
	if (scaled_position().y < scaled_clicked_position().y) {
		adjustment.y = diff.y;
		selection.value().position.y = scaled_position().y;
	}
	selection.value().adjust(adjustment);

	selection.value().dimensions.x = std::clamp(selection.value().dimensions.x, 0u, canvas.dimensions.x - selection.value().position.x);
	selection.value().dimensions.y = std::clamp(selection.value().dimensions.y, 0u, canvas.dimensions.y - selection.value().position.y);

	has_palette_selection = selection.value().get_type() == SelectionType::palette;
}

void Marquee::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {
	if (scancode == sf::Keyboard::Scancode::X) { cut(canvas); }
	if (scancode == sf::Keyboard::Scancode::C) { copy(canvas); }
	if (!clipboard) { return; }
	if (scancode == sf::Keyboard::Scancode::V && !clipboard.value().empty()) { paste(canvas); }
}

void Marquee::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset) {
	sf::RectangleShape box{};
	switch (mode) {
	case SelectMode::clipboard:
		box.setOutlineColor(sf::Color{200, 200, 200, 40});
		box.setFillColor(sf::Color{150, 190, 110, 40});
		box.setOutlineThickness(-2);
		if (clipboard) { box.setSize(clipboard.value().real_dimensions() * canvas.f_cell_size()); }
		if (clipboard) { box.setPosition(f_scaled_position() * canvas.f_cell_size() + offset - clipboard.value().real_dimensions() * canvas.f_cell_size() + sf::Vector2<float>{canvas.f_cell_size(), canvas.f_cell_size()}); }
		if (clipboard && canvas.editable() && !palette_mode) { win.draw(box); }
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

void Marquee::cut(Canvas& canvas) {
	if (!selection) { return; }
	mode = SelectMode::clipboard;
	canvas.save_state(*this, true);
	clipboard = Clipboard(selection.value().dimensions);
	for (uint32_t i = 0; i < selection.value().dimensions.x; ++i) {
		for (uint32_t j = 0; j < selection.value().dimensions.y; ++j) {
			for (auto k{0}; k < canvas.get_layers().layers.size(); ++k) {
				if (!pervasive && k != canvas.active_layer) { continue; }
				clipboard.value().write_to_clipboard(canvas.tile_val_at(selection.value().position.x + i, selection.value().position.y + j, k), i, j, k);
				if (canvas.editable()) { canvas.erase_at(selection.value().position.x + i, selection.value().position.y + j, k); }
			}
		}
	}
}

void Marquee::copy(Canvas& canvas) {
	if (!selection) { return; }
	mode = SelectMode::clipboard;
	clipboard = Clipboard(selection.value().dimensions);
	for (uint32_t i = 0; i < selection.value().dimensions.x; ++i) {
		for (uint32_t j = 0; j < selection.value().dimensions.y; ++j) {
			for (auto k{0}; k < canvas.get_layers().layers.size(); ++k) {
				if (!pervasive && k != canvas.active_layer) { continue; }
				clipboard.value().write_to_clipboard(canvas.tile_val_at(selection.value().position.x + i, selection.value().position.y + j, k), i, j, k);
			}
		}
	}
	std::cout << "copied " << selection.value().dimensions.x * selection.value().dimensions.y << " cells from " << (selection.value().get_type() == SelectionType::canvas ? "canvas" : "palette") << ".\n";
}

void Marquee::paste(Canvas& canvas) {
	if (!clipboard || !selection) { return; }
	if (!canvas.editable()) { return; }
	canvas.save_state(*this, true);
	for (uint32_t i = 0; i < selection.value().dimensions.x; ++i) {
		for (uint32_t j = 0; j < selection.value().dimensions.y; ++j) {
			auto edit_x = scaled_position().x + i - clipboard.value().scaled_dimensions().x + 1;
			auto edit_y = scaled_position().y + j - clipboard.value().scaled_dimensions().y + 1;
			if (edit_x < canvas.dimensions.x && edit_y < canvas.dimensions.y) {
				if (pervasive) {
					for (auto k{0}; k < canvas.get_layers().layers.size(); ++k) { canvas.edit_tile_at(edit_x, edit_y, clipboard.value().get_value_at(i, j, k), k); }
				} else {
					canvas.edit_tile_at(edit_x, edit_y, clipboard.value().get_value_at(i, j), canvas.active_layer);
				}
			}
		}
	}
}

void Marquee::clear() {
	if (!selection) { return; }
	if (!selection.value().empty()) {
		clipboard = Clipboard(selection.value().dimensions);
		selection = {};
	}
}

} // namespace pi
