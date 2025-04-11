
#include "fornani/entities/world/Portal.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/graphics/Transition.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::entity {

Portal::Portal(automa::ServiceProvider& svc, Vecu32 dim, Vecu32 pos, int src, int dest, bool activate_on_contact, bool locked, bool already_open, int key_id, int style, sf::Vector2<int> map_dim)
	: scaled_dimensions(dim), scaled_position(pos), meta({src, dest, key_id}), sprite{svc.assets.get_texture("portals")} {
	dimensions = static_cast<Vec>(dim * util::constants::u32_cell_size);
	position = static_cast<Vec>(pos * util::constants::u32_cell_size);
	bounding_box = shape::Shape(dimensions);
	bounding_box.set_position(position);
	meta.orientation = PortalOrientation::central;
	if (pos.x == 0) { meta.orientation = PortalOrientation::left; }
	if (pos.y == 0) { meta.orientation = PortalOrientation::top; }
	if (pos.x == map_dim.x - 1) { meta.orientation = PortalOrientation::right; }
	if (pos.y == map_dim.y - 1) { meta.orientation = PortalOrientation::bottom; }
	if (already_open) {
		state = PortalRenderState::open;
		flags.attributes.set(PortalAttributes::already_open);
	}
	lookup = sf::IntRect({static_cast<int>(state) * util::constants::i_cell_size, style * util::constants::i_cell_size * 2}, {util::constants::i_cell_size, util::constants::i_cell_size * 2});
	sprite.setTextureRect(lookup);
	if (activate_on_contact) { flags.attributes.set(PortalAttributes::activate_on_contact); }
	if (locked) { flags.state.set(PortalState::locked); }
	if (svc.data.door_is_unlocked(key_id)) { flags.state.reset(PortalState::locked); }
}

void Portal::update(automa::ServiceProvider& svc) {
	position = static_cast<Vec>(scaled_position * util::constants::u32_cell_size);
	dimensions = static_cast<Vec>(scaled_dimensions * util::constants::u32_cell_size);
	bounding_box.set_position(position);
	bounding_box.set_dimensions(dimensions);
	lookup.position.x = static_cast<int>(state) * util::constants::i_cell_size;
}

void Portal::render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos) {
	if (svc.greyblock_mode()) {
		sf::RectangleShape box{};
		if (flags.state.test(PortalState::ready)) {
			box.setFillColor(sf::Color{80, 180, 120, 100});
		} else {
			box.setFillColor(sf::Color{180, 120, 80, 100});
		}
		is_bottom() ? box.setOutlineColor(sf::Color::Blue) : box.setOutlineColor(sf::Color::White);
		box.setOutlineThickness(-1);
		box.setPosition(bounding_box.get_position() - campos);
		box.setSize(dimensions);
		win.draw(box);
	} else if (!flags.attributes.test(PortalAttributes::activate_on_contact)) {
		sprite.setPosition(position - offset - campos);
		sprite.setTextureRect(lookup);
		win.draw(sprite);
	}
}

void Portal::handle_activation(automa::ServiceProvider& svc, player::Player& player, std::optional<std::unique_ptr<gui::Console>>& console, int room_id, flfx::Transition& transition) {
	update(svc);
	if (bounding_box.overlaps(player.collider.bounding_box)) {
		if (flags.attributes.test(PortalAttributes::activate_on_contact) && flags.state.test(PortalState::ready)) {
			flags.state.set(PortalState::activated);
			if (is_left_or_right()) {
				player.controller.prevent_movement();
				player.controller.autonomous_walk();
				player.walk();
			}
		} else if (player.controller.inspecting()) {
			flags.state.set(PortalState::activated);
			player.controller.prevent_movement();
		}
		// player just entered room via border portal
		if (!flags.state.test(PortalState::ready) && flags.attributes.test(PortalAttributes::activate_on_contact) && is_left_or_right()) {
			player.controller.direction.lr = player.entered_from();
			player.controller.prevent_movement();
			player.controller.autonomous_walk();
			player.walk();
		}
	} else {
		if (!flags.state.test(PortalState::ready) && flags.attributes.test(PortalAttributes::activate_on_contact)) { player.controller.stop_walking_autonomously(); }
		flags.state.set(PortalState::ready);
	}
	if (flags.state.test(PortalState::activated)) {
		player.controller.prevent_movement();
		if (!console) {
			if (flags.state.test(PortalState::unlocked)) {
				change_states(svc, room_id, transition);
				flags.state.reset(PortalState::unlocked);
			}
		}
		if (flags.state.test(PortalState::locked)) {
			if (player.has_item(meta.key_id)) {
				flags.state.reset(PortalState::locked);
				flags.state.set(PortalState::unlocked);
				svc.soundboard.flags.world.set(audio::World::door_unlock);
				console = std::make_unique<gui::Console>(svc, svc.text.basic, "unlocked_door", gui::OutputType::gradual);
				console.value()->append(player.catalog.inventory.item_view(meta.key_id).get_label());
				console.value()->display_item(meta.key_id);
				svc.data.unlock_door(meta.key_id);
				svc.soundboard.flags.world.set(audio::World::door_unlock);
			} else {
				console = std::make_unique<gui::Console>(svc, svc.text.basic, "locked_door", gui::OutputType::gradual);
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
		if (svc.data.exists(meta.destination_map_id)) {
			svc.state_controller.next_state = meta.destination_map_id;
		} else {
			svc.state_controller.next_state = meta.source_map_id;
			meta.source_map_id = meta.destination_map_id;
			meta.destination_map_id = room_id;
			svc.state_controller.status.set(automa::Status::out_of_bounds);
		}
		svc.state_controller.actions.set(automa::Actions::trigger);
		svc.state_controller.refresh(meta.source_map_id);
	}
}

} // namespace fornani::entity
