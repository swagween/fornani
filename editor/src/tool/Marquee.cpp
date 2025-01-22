
#include "editor/tool/Tool.hpp"

namespace pi {

void Marquee::handle_events(Canvas& canvas, sf::Event& e) {
	sf::Vector2<uint32_t> dim = {static_cast<uint32_t>(canvas.get_real_dimensions().x), static_cast<uint32_t>(canvas.get_real_dimensions().y)};
	if (in_bounds(dim) && ready) {
		if (active) {
			if (just_clicked) {
				clicked_position = position;
				selection = SelectBox(scaled_clicked_position(), {});
				clipboard = {};
				mode = SelectMode::select;
				just_clicked = false;
			}
			// I'm being long-winded here to provision for the mouse
			// being dragged above or left of the clicked position
			auto tweak = 1;
			sf::Vector2<uint32_t> adjustment{};
			if (scaled_position().x >= scaled_clicked_position().x) {
				adjustment.x = scaled_position().x + tweak - selection.position.x;
			} else {
				adjustment.x = scaled_clicked_position().x - scaled_position().x;
				selection.position.x = scaled_position().x + tweak;
			}
			if (scaled_position().y >= scaled_clicked_position().y) {
				adjustment.y = scaled_position().y + tweak - selection.position.y;
			} else {
				adjustment.y = scaled_clicked_position().y - scaled_position().y;
				selection.position.y = scaled_position().y + tweak;
			}
			selection.adjust(adjustment);
			if (!selection.empty()) {
				clipboard = Clipboard(selection.dimensions);
				if (!canvas.editable()) { has_palette_selection = true; }
			}
		}
	}
	update();
}

void Marquee::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key) {
	if (!clipboard) { return; }
	if (key == sf::Keyboard::X) { cut(canvas); }
	if (key == sf::Keyboard::C) { copy(canvas); }
	if (key == sf::Keyboard::V && !clipboard.value().empty()) { paste(canvas); }
}

void Marquee::update() { Tool::update(); }

void Marquee::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed) {
	sf::RectangleShape box{};
	switch (mode) {
	case SelectMode::clipboard:
		box.setOutlineColor(sf::Color{200, 200, 200, 40});
		box.setFillColor(sf::Color{150, 190, 110, 40});
		box.setOutlineThickness(-2);
		if (clipboard) { box.setSize(clipboard.value().real_dimensions() * canvas.f_cell_size()); }
		if (clipboard) { box.setPosition(f_scaled_position() * canvas.f_cell_size() + offset - clipboard.value().real_dimensions() * canvas.f_cell_size() + sf::Vector2<float>{canvas.f_cell_size(), canvas.f_cell_size()}); }
		if (transformed) { win.draw(box); }
		[[fallthrough]];
	case SelectMode::select:
		box.setOutlineColor(sf::Color{100, 255, 160, 180});
		box.setFillColor(sf::Color{110, 100, 180, 40});
		box.setOutlineThickness(-2);
		box.setSize({selection.dimensions.x * canvas.f_cell_size(), selection.dimensions.y * canvas.f_cell_size()});
		box.setPosition(selection.f_position() * canvas.f_cell_size() + offset);
		if (!has_palette_selection || !transformed) { win.draw(box); }
		break;
	}
}

void Marquee::store_tile(int index) {}

void Marquee::cut(Canvas& canvas) {
	if (!clipboard) { return; }
	mode = SelectMode::clipboard;
	canvas.save_state(*this, true);
	clipboard.value().clear_clipboard();
	for (uint32_t i = 0; i < selection.dimensions.x; ++i) {
		for (uint32_t j = 0; j < selection.dimensions.y; ++j) {
			for (auto k{0}; k < canvas.get_layers().layers.size(); ++k) {
				if (!pervasive && k != canvas.active_layer) { continue; }
				clipboard.value().write_to_clipboard(canvas.tile_val_at(selection.position.x + i, selection.position.y + j, k), i, j, k);
				if (canvas.editable()) { canvas.erase_at(selection.position.x + i, selection.position.y + j, k); }
			}
		}
	}
}

void Marquee::copy(Canvas& canvas) {
	if (!clipboard) { return; }
	mode = SelectMode::clipboard;
	clipboard.value().clear_clipboard();
	for (uint32_t i = 0; i < selection.dimensions.x; ++i) {
		for (uint32_t j = 0; j < selection.dimensions.y; ++j) {
			for (auto k{0}; k < canvas.get_layers().layers.size(); ++k) {
				if (!pervasive && k != canvas.active_layer) { continue; }
				clipboard.value().write_to_clipboard(canvas.tile_val_at(selection.position.x + i, selection.position.y + j, k), i, j, k);
			}
		}
	}
}

void Marquee::paste(Canvas& canvas) {
	if (!clipboard) { return; }
	if (!canvas.editable()) { return; }
	canvas.save_state(*this, true);
	for (uint32_t i = 0; i < selection.dimensions.x; ++i) {
		for (uint32_t j = 0; j < selection.dimensions.y; ++j) {
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
	if (!selection.empty()) { clipboard = Clipboard(selection.dimensions); }
}

} // namespace pi
