
#include "Portal.hpp"
#include "../player/Player.hpp"
#include "../../gui/Console.hpp"
#include "../../service/ServiceProvider.hpp"

namespace entity {

Portal::Portal(automa::ServiceProvider& svc, Vecu32 dim, Vecu32 pos, int src, int dest, bool activate_on_contact, bool locked, bool already_open, int key_id) : scaled_dimensions(dim), scaled_position(pos), meta({src, dest, key_id}) {
	dimensions = static_cast<Vec>(dim * svc.constants.u32_cell_size);
	position = static_cast<Vec>(pos * svc.constants.u32_cell_size);
	bounding_box = shape::Shape(dimensions);
	bounding_box.set_position(position);
	sprite.setTexture(svc.assets.t_portals);
	if (already_open) {
		state = PortalRenderState::open;
		flags.attributes.set(PortalAttributes::already_open);
	}
	lookup = sf::IntRect({static_cast<int>(state) * svc.constants.i_cell_size, 0}, {svc.constants.i_cell_size, svc.constants.i_cell_size * 2});
	sprite.setTextureRect(lookup);
	if (activate_on_contact) { flags.attributes.set(PortalAttributes::activate_on_contact); }
	if (locked) { flags.state.set(PortalState::locked); }
	if (svc.data.door_is_unlocked(key_id)) { flags.state.reset(PortalState::locked); }
}

void Portal::update(automa::ServiceProvider& svc) {
	position = static_cast<Vec>(scaled_position * svc.constants.u32_cell_size);
	dimensions = static_cast<Vec>(scaled_dimensions * svc.constants.u32_cell_size);
	bounding_box.set_position(position);
	bounding_box.dimensions = dimensions;
	lookup.left = static_cast<int>(state) * svc.constants.i_cell_size;
}

void Portal::render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos) {
	if (svc.greyblock_mode()) {
		sf::RectangleShape box{};
		if (flags.state.test(PortalState::ready)) {
			box.setFillColor(sf::Color{80, 180, 120, 100});
		} else {
			box.setFillColor(sf::Color{180, 120, 80, 100});
		}
		box.setOutlineColor(sf::Color::White);
		box.setOutlineThickness(-1);
		box.setPosition(bounding_box.position - campos);
		box.setSize(dimensions);
		win.draw(box);
	} else if (!flags.attributes.test(PortalAttributes::activate_on_contact)) {
		sprite.setPosition(position - offset - campos);
		sprite.setTextureRect(lookup);
		win.draw(sprite);
	}
}

void Portal::handle_activation(automa::ServiceProvider& svc, player::Player& player, gui::Console& console, int room_id, flfx::Transition& transition) {
	update(svc);
	if (bounding_box.overlaps(player.collider.bounding_box)) {
		if (flags.attributes.test(PortalAttributes::activate_on_contact) && flags.state.test(PortalState::ready)) {
			flags.state.set(PortalState::activated);
			player.controller.prevent_movement();
			player.controller.autonomous_walk();
			player.walk();
		} else if (player.controller.inspecting()) {
			flags.state.set(PortalState::activated);
			player.controller.prevent_movement();
		}
		// player just entered room via border portal
		if (!flags.state.test(PortalState::ready) && flags.attributes.test(PortalAttributes::activate_on_contact)) {
			player.controller.direction.lr = player.entered_from();
			player.controller.prevent_movement();
			player.controller.autonomous_walk();
			player.walk();
		}
	} else {
		if (!flags.state.test(PortalState::ready) && flags.attributes.test(PortalAttributes::activate_on_contact)) {
			player.controller.stop_walking_autonomously();
		}
		flags.state.set(PortalState::ready);
	}
	if (flags.state.test(PortalState::activated)) {
		player.controller.prevent_movement();
		if (console.is_complete()) {
			if (flags.state.test(PortalState::unlocked)) {
				change_states(svc, room_id, transition);
				flags.state.reset(PortalState::unlocked);
			}
		}
		if (flags.state.test(PortalState::locked)) {
			console.set_source(svc.text.basic);
			if (player.has_item(meta.key_id)) {
				flags.state.reset(PortalState::locked);
				flags.state.set(PortalState::unlocked);
				svc.soundboard.flags.world.set(audio::World::door_unlock);
				console.load_and_launch("unlocked_door");
				console.append(player.catalog.categories.inventory.get_item(meta.key_id).get_label());
				console.display_item(meta.key_id);
				svc.data.unlock_door(meta.key_id);
				svc.soundboard.flags.world.set(audio::World::door_unlock);
			} else {
				console.load_and_launch("locked_door");
				flags.state.reset(PortalState::activated);
			}
			return;
		}
		if (flags.state.test(PortalState::unlocked)) { return; }
		change_states(svc, room_id, transition);
	}
}

void Portal::change_states(automa::ServiceProvider& svc, int room_id, flfx::Transition& transition) {
	if (!flags.attributes.test(PortalAttributes::activate_on_contact) && transition.not_started()) {
		state = PortalRenderState::open;
		if (!flags.attributes.test(PortalAttributes::already_open)) { svc.soundboard.flags.world.set(audio::World::door_open); }
	}
	transition.start();
	if (transition.is_done()) {
		try {
			svc.state_controller.next_state = meta.destination_map_id;
		} catch (std::out_of_range) { svc.state_controller.next_state = room_id; }
		svc.state_controller.actions.set(automa::Actions::trigger);
		svc.state_controller.refresh(meta.source_map_id);
	}
}

} // namespace entity
