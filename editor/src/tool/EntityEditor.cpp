#include "editor/tool/Tool.hpp"

namespace pi {

EntityEditor::EntityEditor(EntityMode to_mode) {
	type = ToolType::entity_editor;
	set_mode(to_mode);
	ent_type = EntityType::none;
}

void EntityEditor::handle_events(Canvas& canvas, sf::Event& e) {
	if (in_bounds(canvas.dimensions) && ready && canvas.editable()) {
		if (selector_mode()) {
			ent_type = EntityType::none;
		}
		if (placer_mode()) {
			if (current_entity) {
				canvas.entities.variables.entities.push_back(std::move(current_entity.value()));
				current_entity = {}; // free the entity's memory
			}
		}
		if (eraser_mode()) {
			ent_type = EntityType::none;
			std::erase_if(canvas.entities.variables.entities, [this](auto& e) { return e->position == scaled_position(); });
		}
		if (mover_mode()) {}
	}
	update();
}

void EntityEditor::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key) {
	if (key == sf::Keyboard::Q) { trigger_switch = true; }
}

void EntityEditor::update() {
	Tool::update();
	if (!current_entity) { return; }
	current_entity.value()->position = scaled_position() - current_entity.value()->dimensions + sf::Vector2<uint32_t>(1, 1);
}

void EntityEditor::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed) {
	sf::RectangleShape box{};

	box.setOutlineColor(sf::Color{200, 200, 200, 80});
	box.setFillColor(sf::Color{100, 190, 190, 80});
	box.setOutlineThickness(-2);
	box.setSize({canvas.f_cell_size(), canvas.f_cell_size()});

	if (!current_entity) { return; }
	if (ent_type == EntityType::portal) {
		for (uint32_t i = 0; i < current_entity.value()->dimensions.x; ++i) {
			for (uint32_t j = 0; j < current_entity.value()->dimensions.y; ++j) {
				box.setPosition(current_entity.value()->position.x * canvas.f_cell_size() + i * canvas.f_cell_size() + offset.x, current_entity.value()->position.y * canvas.f_cell_size() + j * canvas.f_cell_size() + offset.y);
				win.draw(box);
			}
		}
	} else if (ent_type == EntityType::animator) {
		for (uint32_t i = 0; i < current_entity.value()->dimensions.x; ++i) {
			for (uint32_t j = 0; j < current_entity.value()->dimensions.y; ++j) {
				box.setPosition(current_entity.value()->position.x * canvas.f_cell_size() + i * canvas.f_cell_size() + offset.x, current_entity.value()->position.y * canvas.f_cell_size() + j * canvas.f_cell_size() + offset.y);
				win.draw(box);
			}
		}
	} else {
		box.setPosition(scaled_position().x * canvas.f_cell_size() + offset.x, scaled_position().y * canvas.f_cell_size() + offset.y);
		win.draw(box);
	}
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
