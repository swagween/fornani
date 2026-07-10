
#include "editor/tool/Tool.hpp"

namespace pi {

void Erase::update(Canvas& canvas) {
	Tool::update(canvas);
	if (just_clicked) { just_clicked = false; }
	if (in_bounds(canvas.dimensions) && active && canvas.editable() && selection_type == canvas.get_selection_type()) {
		if (is_mode(BrushMode::tile)) {
			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) { canvas.erase_at(scaled_position().x - i, scaled_position().y - j, canvas.active_layer); }
			}
		} else if (is_mode(BrushMode::hazard)) {
			canvas.erase_hazard_at(scaled_position());
		}
	}
}

void Erase::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {}

void Erase::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2f offset) {}

void Erase::store_tile(int index) { tile = index; }

void Erase::clear() {}

} // namespace pi
