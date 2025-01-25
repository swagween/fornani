
#include "editor/tool/Tool.hpp"
#include <algorithm>

namespace pi {

bool Tool::in_bounds(sf::Vector2<uint32_t>& bounds) const { return scaled_position().x >= 0 && scaled_position().x < bounds.x && scaled_position().y >= 0 && scaled_position().y < bounds.y; }

void Tool::set_position(sf::Vector2<float> to_position) { position = to_position; }

void Tool::set_window_position(sf::Vector2<float> to_position) { window_position = to_position; }

void Tool::activate() { active = true; }

void Tool::deactivate() { active = false; }

void Tool::suppress_until_released() { ready = false; }

void Tool::unsuppress() { ready = true; }

void Tool::neutralize() { selection_type = SelectionType::neutral; }

void Tool::click() { just_clicked = true; }

void Tool::release() { just_released = true; }

void Tool::change_size(int amount) { size = std::clamp(size + amount, 1, max_size); }

void Tool::update(Canvas& canvas) {
	if (just_clicked) { selection_type = canvas.editable() ? SelectionType::canvas : SelectionType::palette; }
}

void Tool::set_usability(bool const flag) {
	if (type == ToolType::hand) {
		// hand tool is always usable
		status = ToolStatus::usable;
		return;
	}
	status = flag ? ToolStatus::usable : ToolStatus::unusable;
}

} // namespace pi
