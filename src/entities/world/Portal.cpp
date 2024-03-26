
#include "Portal.hpp"
#include "../../setup/MapLookups.hpp"
#include "../../setup/ServiceLocator.hpp"
#include "../player/Player.hpp"
#include "../../service/ServiceProvider.hpp"

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
	*/
}

void Portal::handle_activation(automa::ServiceProvider& svc, player::Player& player, int room_id, bool& fade_out, bool& done) {

	if (bounding_box.overlaps(player.collider.bounding_box)) {
		if (activate_on_contact && ready) {
			activated = true;
			player.controller.prevent_movement();
			player.controller.autonomous_walk();
			player.walk();
		} else if (player.controller.inspecting()) {
			activated = true;
			player.controller.prevent_movement();
		}
		// player just entered room via border portal
		if (!ready && activate_on_contact) {
			player.controller.direction.lr = player.entered_from();
			player.controller.prevent_movement();
			player.controller.autonomous_walk();
			player.walk();
		}
	} else {
		if (!ready && activate_on_contact) {
			player.controller.stop_walking_autonomously();
		}
		ready = true;
	}
	if (activated) {
		fade_out = true;
		if (done) {
			try {
				svc.state_controller.next_state = lookup::get_map_label.at(destination_map_id);
			} catch (std::out_of_range) { svc.state_controller.next_state = lookup::get_map_label.at(room_id); }
			svc.state_controller.actions.set(automa::Actions::trigger);
			svc.state_controller.refresh(source_map_id);
		}
	}
}

} // namespace entity
