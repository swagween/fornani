#include "editor/tool/Tool.hpp"

namespace pi {

EntityEditor::EntityEditor(EntityMode to_mode) : Tool("Entity Editor", ToolType::entity_editor) {
	set_mode(to_mode);
	ent_type = EntityType::none;
}

void EntityEditor::update(Canvas& canvas) {
	Tool::update(canvas);

	// set tooltip
	switch (entity_mode) {
	case EntityMode::selector: tooltip = "Selector"; break;
	case EntityMode::editor: tooltip = "Editor"; break;
	case EntityMode::placer: tooltip = "Placer (press Q to cancel)"; break;
	case EntityMode::eraser: tooltip = "Eraser"; break;
	case EntityMode::mover: tooltip = "Mover"; break;
	}

	if (just_clicked) { just_clicked = false; }
	disable_highlight = true;

	if (entity_menu) {
		if (active && !canvas.entities.has_entity_at(scaled_position(), true)) {
			current_entity = {};
			entity_mode = EntityMode::selector;
			entity_menu = false;
		}
	}

	if (selector_mode()) {
		ent_type = EntityType::none;
		for (auto& ent : canvas.entities.variables.entities) {
			if (!entity_menu || active) { ent->highlighted = ent->contains_point(scaled_position()); }
			if (active && ent->highlighted && is_ready()) {
				if (ent->copyable) { current_entity = ent->clone(); }
				entity_menu = true;
			}
		}
	}

	if (editor_mode()) {
		// user just saved changes
		if (!is_ready()) {
			unsuppress();
			entity_mode = EntityMode::selector;
		}
		// user clicks away
		if (active && !canvas.entities.has_entity_at(scaled_position(), true)) {
			current_entity = {};
			entity_mode = EntityMode::selector;
			return;
		}
		// edit mode
		if (!is_ready()) { current_entity = {}; }
		for (auto& ent : canvas.entities.variables.entities) {
			if (ent->overwrite) {
				if (current_entity) {
					ent = std::move(current_entity.value());
					current_entity = {};
					break;
				}
			}
		}
	}

	if (placer_mode() && is_ready() && canvas.editable()) {
		disable_highlight = false;
		if (current_entity) { current_entity.value()->set_position(scaled_position() - current_entity.value()->get_grid_dimensions() + sf::Vector2<std::uint32_t>(1, 1)); }
		// user duplicated an existing entity
		if (!current_entity) {
			for (auto& ent : canvas.entities.variables.entities) {
				if (ent->highlighted) { current_entity = ent->clone(); }
			}
		}
		if (ent_type == EntityType::player_placer) {
			if (active) { canvas.entities.variables.player_start = scaled_position(); }
		} else if (current_entity && active) {
			if (!canvas.entities.overlaps(*current_entity.value())) {
				auto repeat = current_entity.value()->repeatable && !current_entity.value()->moved;
				auto clone = current_entity.value()->clone();
				canvas.entities.variables.entities.push_back(std::move(current_entity.value()));
				if (repeat) {
					current_entity = std::move(clone);
				} else {
					current_entity = {}; // free the entity's memory otherwise
					entity_mode = EntityMode::selector;
				}
				suppress_until_released();
			}
		}
	}

	if (eraser_mode()) {
		ent_type = EntityType::none;
		std::erase_if(canvas.entities.variables.entities, [this](auto& e) { return e->highlighted; });
		entity_mode = EntityMode::selector;
	}

	if (mover_mode()) {
		for (auto& ent : canvas.entities.variables.entities) {
			if (ent->highlighted) {
				current_entity = ent->clone();
				current_entity.value()->moved = true;
			}
		}
		std::erase_if(canvas.entities.variables.entities, [this](auto& e) { return e->highlighted; });
		entity_mode = EntityMode::placer;
	}
}

void EntityEditor::handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) {
	if (scancode == sf::Keyboard::Scancode::Q) {
		current_entity = {}; // free the entity's memory otherwise
		entity_mode = EntityMode::selector;
	}
}

void EntityEditor::render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2f offset) {
	if (!canvas.editable()) { return; }
	if (!current_entity) { return; }

	if (!disable_highlight) {
		sf::RectangleShape box{};
		box.setOutlineColor(sf::Color{200, 200, 200, 80});
		box.setFillColor(sf::Color{100, 190, 190, 80});
		box.setOutlineThickness(-2);
		box.setSize({current_entity.value()->get_grid_dimensions().x * canvas.f_cell_size(), current_entity.value()->get_grid_dimensions().y * canvas.f_cell_size()});
		box.setPosition(
			{(scaled_position().x - current_entity.value()->get_grid_dimensions().x + 1) * canvas.f_cell_size() + offset.x, (scaled_position().y - current_entity.value()->get_grid_dimensions().y + 1) * canvas.f_cell_size() + offset.y});
		win.draw(box);
	}
}

void EntityEditor::store_tile(int index) {}

void EntityEditor::clear() {}

void EntityEditor::set_mode(EntityMode to_mode) { entity_mode = to_mode; }

void EntityEditor::set_usability(bool const flag) {
	if (placer_mode()) { status = ToolStatus::loaded; }
	if (selector_mode()) { status = ToolStatus::usable; }
}

} // namespace pi
