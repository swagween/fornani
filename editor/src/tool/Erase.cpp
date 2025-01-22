//
//  Erase.cpp
//  Pioneer-Lab
//
//  Created by Alex Frasca on 10/3/20.
//

#include "editor/tool/Tool.hpp"

namespace pi {

void Erase::handle_events(Canvas& canvas, sf::Event& e) {
	if (in_bounds(canvas.dimensions) && ready && canvas.editable()) {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) { canvas.erase_at(scaled_position().x - i, scaled_position().y - j, canvas.active_layer); }
		}
	}
	update();
}

void Erase::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key) {}

void Erase::update() { Tool::update(); }

void Erase::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed) {}

void Erase::store_tile(int index) { tile = index; }

void Erase::clear() {}

} // namespace pi
