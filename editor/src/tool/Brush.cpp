
#include "editor/tool/Tool.hpp"

namespace pi {

void Brush::update(Canvas& canvas) {
	Tool::update(canvas);
	if (just_clicked) { just_clicked = false; }
	if (in_bounds(canvas.dimensions) && active && canvas.editable() && selection_type == canvas.get_selection_type()) {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) { canvas.edit_tile_at(scaled_position().x - i, scaled_position().y - j, tile, canvas.active_layer); }
		}
	}
}

void Brush::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {}

void Brush::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset) {}

void Brush::store_tile(int index) { tile = index; }

void Brush::clear() {}

} // namespace pi
