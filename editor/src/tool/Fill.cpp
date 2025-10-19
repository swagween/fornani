
#include "editor/tool/Tool.hpp"

namespace pi {

void Fill::update(Canvas& canvas) {
	Tool::update(canvas);
	if (just_clicked) { just_clicked = false; }
	if (in_bounds(canvas.dimensions) && active && canvas.editable() && selection_type == canvas.get_selection_type()) {
		if (contiguous) {
			fill_section(canvas.tile_val_at(scaled_position().x, scaled_position().y, canvas.active_layer), tile, scaled_position().x, scaled_position().y, canvas);
		} else {
			replace_all(canvas.tile_val_at(scaled_position().x, scaled_position().y, canvas.active_layer), tile, scaled_position().x, scaled_position().y, canvas);
		}
	}
}

void Fill::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {}

void Fill::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2f offset) {}

void Fill::fill_section(std::uint32_t const prev_val, std::uint32_t const new_val, std::uint32_t i, std::uint32_t j, Canvas& canvas) {
	if (!canvas.editable()) { return; }
	if (i < 0 || i >= canvas.dimensions.x || j < 0 || j >= canvas.dimensions.y) {
		return;
	} else if (canvas.tile_val_at(i, j, canvas.active_layer) != prev_val) {
		return;
	} else if (canvas.tile_val_at(i, j, canvas.active_layer) == new_val) {
		return;
	} else {

		canvas.edit_tile_at(i, j, new_val, canvas.active_layer);

		fill_section(prev_val, new_val, i + 1, j, canvas);
		fill_section(prev_val, new_val, i - 1, j, canvas);
		fill_section(prev_val, new_val, i, j + 1, canvas);
		fill_section(prev_val, new_val, i, j - 1, canvas);
	}
}

void Fill::replace_all(std::uint32_t const prev_val, std::uint32_t const new_val, std::uint32_t i, std::uint32_t j, Canvas& canvas) {
	if (pervasive) {
		for (auto k{0}; k < canvas.get_layers().layers.size(); ++k) { canvas.replace_tile(prev_val, new_val, k); }
	} else {
		canvas.replace_tile(prev_val, new_val, canvas.active_layer);
	}
}

void Fill::store_tile(int index) { tile = index; }

void Fill::clear() {}

} // namespace pi
