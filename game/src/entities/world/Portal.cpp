
#include "fornani/entities/world/Portal.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/graphics/Transition.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::entity {

Portal::Portal(automa::ServiceProvider& svc, sf::Vector2<std::uint32_t> dim, sf::Vector2<std::uint32_t> pos, int src, int dest, bool activate_on_contact, bool locked, bool already_open, int key_id, int style, sf::Vector2i map_dim)
	: IWorldPositionable(pos, dim), Drawable(svc, "portals"), meta({src, dest, key_id}), m_style(style) {
	bounding_box = shape::Shape(get_world_dimensions());
	bounding_box.set_position(get_world_position());
	meta.orientation = PortalOrientation::central;
	if (pos.x == 0) { meta.orientation = PortalOrientation::left; }
	if (pos.y == 0) { meta.orientation = PortalOrientation::top; }
	if (pos.x == map_dim.x - 1) { meta.orientation = PortalOrientation::right; }
	if (pos.y == map_dim.y - 1) { meta.orientation = PortalOrientation::bottom; }
	if (already_open) {
		state = PortalRenderState::open;
		flags.attributes.set(PortalAttributes::already_open);
	}
	auto lookup = sf::IntRect({static_cast<int>(state) * constants::i_cell_resolution, style * constants::i_cell_resolution * 2}, {constants::i_cell_resolution, constants::i_cell_resolution * 2});
	set_texture_rect(lookup);
	if (activate_on_contact) { flags.attributes.set(PortalAttributes::activate_on_contact); }
	if (locked) { flags.state.set(PortalState::locked); }
	if (svc.data.door_is_unlocked(key_id)) { flags.state.reset(PortalState::locked); }
}

void Portal::update(automa::ServiceProvider& svc) {
	auto lookup = sf::IntRect({static_cast<int>(state) * constants::i_cell_resolution, m_style * constants::i_cell_resolution * 2}, {constants::i_cell_resolution, constants::i_cell_resolution * 2});
	set_texture_rect(lookup);
}

void Portal::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f const cam) {
	if (svc.greyblock_mode()) {
		sf::RectangleShape box{};
		if (flags.state.test(PortalState::ready)) {
			box.setFillColor(sf::Color{80, 180, 120, 100});
		} else {
			box.setFillColor(sf::Color{180, 120, 80, 100});
		}
		is_bottom() ? box.setOutlineColor(sf::Color::Blue) : box.setOutlineColor(sf::Color::White);
		box.setOutlineThickness(-1);
		box.setPosition(bounding_box.get_position() - cam);
		box.setSize(get_world_dimensions());
		win.draw(box);
	} else if (!flags.attributes.test(PortalAttributes::activate_on_contact)) {
		auto offset = sf::Vector2f{0.f, constants::f_cell_size};
		set_position(get_world_position() - offset - cam);
		win.draw(*this);
	}
}

void Portal::handle_activation(automa::ServiceProvider& svc, player::Player& player, std::optional<std::unique_ptr<gui::Console>>& console, int room_id, graphics::Transition& transition) {
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
			player.controller.direction.set_from_simple(player.entered_from());
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
				console.value()->append(player.catalog.inventory.item_view(meta.key_id).get_title());
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

void Portal::change_states(automa::ServiceProvider& svc, int room_id, graphics::Transition& transition) {
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
