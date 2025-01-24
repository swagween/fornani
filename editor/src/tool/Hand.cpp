

#include "editor/tool/Tool.hpp"

namespace pi {

void Hand::update(Canvas& canvas) {
	Tool::update(canvas);
	if (active) {
		if (just_clicked) {
			clicked_position = window_position;
			just_clicked = false;
		}
		auto translation = window_position - clicked_position;
		canvas.move(translation);
		clicked_position = window_position;
	} else {
		just_clicked = true;
	}
}

void Hand::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {}

void Hand::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset) {}

void Hand::store_tile(int index) {}

void Hand::clear() {}

} // namespace pi
