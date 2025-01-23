

#include "editor/tool/Tool.hpp"

namespace pi {

void Hand::handle_events(Canvas& canvas) {
	if (active) {
		if (just_clicked) {
			clicked_position = window_position;
			just_clicked = false;
		}
		auto translation = window_position - clicked_position;
		canvas.move(translation);
		clicked_position = window_position;
	}
}

void Hand::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {}

void Hand::update() { Tool::update(); }

void Hand::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed) {}

void Hand::store_tile(int index) {}

void Hand::clear() {}

} // namespace pi
