//

#include "editor/tool/Tool.hpp"

namespace pi {

void Eyedropper::handle_events(Canvas& canvas, sf::Event& e) {
	if (in_bounds(canvas.dimensions) && ready) { tile = canvas.tile_val_at(scaled_position().x, scaled_position().y, canvas.active_layer); }
	update();
}

void Eyedropper::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key) {}

void Eyedropper::update() { Tool::update(); }

void Eyedropper::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed) {}

void Eyedropper::store_tile(int index) { tile = index; }

void Eyedropper::clear() {}

} // namespace pi
