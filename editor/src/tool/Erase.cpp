//
//  Erase.cpp
//  Pioneer-Lab
//
//  Created by Alex Frasca on 10/3/20.
//

#include "editor/tool/Tool.hpp"

namespace pi {

void Erase::update(Canvas& canvas) {
	Tool::update(canvas);
	if (in_bounds(canvas.dimensions) && active && canvas.editable()) {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) { canvas.erase_at(scaled_position().x - i, scaled_position().y - j, canvas.active_layer); }
		}
	}
}

void Erase::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {}

void Erase::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset) {}

void Erase::store_tile(int index) { tile = index; }

void Erase::clear() {}

} // namespace pi
