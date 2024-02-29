
#include "Portal.hpp"
#include "../../setup/MapLookups.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace entity {

void Portal::update() {
	position = static_cast<Vec>(scaled_position * CELL_SIZE);
	dimensions = static_cast<Vec>(scaled_dimensions * CELL_SIZE);
	bounding_box.set_position(position);
	bounding_box.dimensions = dimensions;
}

void Portal::render(sf::RenderWindow& win, Vec campos) {
	sf::RectangleShape box{};
	if (ready) {
		box.setFillColor(sf::Color{80, 180, 120, 100});
	} else {
		box.setFillColor(sf::Color{180, 120, 80, 100});
	}
	box.setOutlineColor(sf::Color::White);
	box.setOutlineThickness(-1);
	box.setPosition(bounding_box.position - campos);
	box.setSize(dimensions);
	/*win.draw(box);
	svc::counterLocator.get().at(svc::draw_calls)++;*/
}

void Portal::handle_activation(int room_id, bool& fade_out, bool& done) {

	if (bounding_box.SAT(svc::playerLocator.get().collider.bounding_box)) {
		if (activate_on_contact && ready) {
			activated = true;
			svc::playerLocator.get().controller.prevent_movement();
			svc::playerLocator.get().controller.autonomous_walk();
			svc::playerLocator.get().walk();
		} else if (svc::playerLocator.get().controller.inspecting()) {
			activated = true;
			svc::playerLocator.get().controller.prevent_movement();
		}
		// player just entered room via border portal
		if (!ready && activate_on_contact) {
			svc::playerLocator.get().controller.direction.lr = svc::playerLocator.get().entered_from();
			svc::playerLocator.get().controller.prevent_movement();
			svc::playerLocator.get().controller.autonomous_walk();
			svc::playerLocator.get().walk();
		}
	} else {
		if (!ready && activate_on_contact) {
			svc::playerLocator.get().unrestrict_inputs();
			svc::playerLocator.get().controller.stop_walking_autonomously();
		}
		ready = true;
	}
	if (activated) {
		fade_out = true;
		if (done) {
			try {
				svc::stateControllerLocator.get().next_state = lookup::get_map_label.at(destination_map_id);
			} catch (std::out_of_range) { svc::stateControllerLocator.get().next_state = lookup::get_map_label.at(room_id); }
			svc::stateControllerLocator.get().trigger = true;
			svc::stateControllerLocator.get().source_id = source_map_id;
		}
	}
}

} // namespace entity
