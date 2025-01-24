#include "editor/tool/Tool.hpp"

namespace pi {

EntityEditor::EntityEditor(EntityMode to_mode) {
	type = ToolType::entity_editor;
	set_mode(to_mode);
	ent_type = EntityType::none;
}

void EntityEditor::update(Canvas& canvas) {
	Tool::update(canvas);
	if (just_clicked) { just_clicked = false; }
	if (current_entity) { current_entity.value()->position = scaled_position() - current_entity.value()->dimensions + sf::Vector2<uint32_t>(1, 1); }
	if (!in_bounds(canvas.dimensions) || !active || !canvas.editable()) { return; }
	if (selector_mode()) { ent_type = EntityType::none; }
	if (placer_mode() && is_ready()) {
		if (current_entity) {
			auto repeat = current_entity.value()->repeatable;
			auto clone = current_entity.value()->clone();
			canvas.entities.variables.entities.push_back(std::move(current_entity.value()));
			if (repeat) {
				current_entity = std::move(clone);
			} else {
				current_entity = {}; // free the entity's memory otherwise
			}
			suppress_until_released();
		}
	}
	if (eraser_mode()) {
		ent_type = EntityType::none;
		std::erase_if(canvas.entities.variables.entities, [this](auto& e) { return e->position == scaled_position(); });
	}
	if (mover_mode()) {}
}

void EntityEditor::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {
	if (scancode == sf::Keyboard::Scancode::Q) { trigger_switch = true; }
}

void EntityEditor::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset) {
	if (!canvas.editable()) { return; }
	if (!current_entity) { return; }

	sf::RectangleShape box{};
	box.setOutlineColor(sf::Color{200, 200, 200, 80});
	box.setFillColor(sf::Color{100, 190, 190, 80});
	box.setOutlineThickness(-2);
	box.setSize({current_entity.value()->dimensions.x * canvas.f_cell_size(), current_entity.value()->dimensions.y * canvas.f_cell_size()});
	box.setPosition({(scaled_position().x - current_entity.value()->dimensions.x + 1) * canvas.f_cell_size() + offset.x, (scaled_position().y - current_entity.value()->dimensions.y + 1) * canvas.f_cell_size() + offset.y});
	win.draw(box);
}

void EntityEditor::store_tile(int index) {}

void EntityEditor::clear() {}

void EntityEditor::set_mode(EntityMode to_mode) {
	entity_mode = to_mode;
	switch (to_mode) {
	case EntityMode::selector: tooltip = "Selector"; break;
	case EntityMode::placer: tooltip = "Placer"; break;
	case EntityMode::eraser: tooltip = "Eraser"; break;
	case EntityMode::mover: tooltip = "Mover"; break;
	}
}

void EntityEditor::set_usability(bool const flag) {
	if (eraser_mode()) { status = ToolStatus::destructive; }
}

} // namespace pi
