
#include "editor/tool/Tool.hpp"

namespace pi {

void Brush::handle_events(Canvas& canvas) {
	if (in_bounds(canvas.dimensions) && ready && canvas.editable()) {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) { canvas.edit_tile_at(scaled_position().x - i, scaled_position().y - j, tile, canvas.active_layer); }
		}
	}
	update();
}

void Brush::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {}

void Brush::update() { Tool::update(); }

void Brush::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed) {}

void Brush::store_tile(int index) { tile = index; }

void Brush::clear() {}

} // namespace pi
