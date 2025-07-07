//

#include "editor/tool/Tool.hpp"

namespace pi {

void Eyedropper::update(Canvas& canvas) {
	Tool::update(canvas);
	if (in_bounds(canvas.dimensions) && active) { tile = canvas.tile_val_at(scaled_position().x, scaled_position().y, canvas.active_layer); }
}

void Eyedropper::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {}

void Eyedropper::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2f offset) {}

void Eyedropper::store_tile(int index) { tile = index; }

void Eyedropper::clear() {}

} // namespace pi
