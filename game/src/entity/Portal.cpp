
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/Portal.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Portal::Portal(automa::ServiceProvider& svc, sf::Vector2u dimensions, bool activate_on_contact, bool already_open, int source_id, int destination_id, bool locked, int key_id)
	: Entity(svc, "portals", 0, dimensions), source_id(source_id), destination_id(destination_id), key_id(key_id), m_services(&svc) {
	set_texture_rect(sf::IntRect{{16 * already_open, 0}, {16, 32}});
	set_origin({0.f, 16.f});
	if (activate_on_contact || dimensions.x * dimensions.y > 1) { m_textured = false; }
	if (activate_on_contact) { m_attributes.set(PortalAttributes::activate_on_contact); }
	if (already_open) { m_attributes.set(PortalAttributes::already_open); }
}

Portal::Portal(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "portals"), m_services(&svc) {
	unserialize(in);
	if (is_activate_on_contact()) { m_textured = false; }
	set_origin({0.f, 16.f});
	bounding_box = shape::Shape(get_world_dimensions());
	bounding_box.set_position(get_world_position());
	m_orientation = PortalOrientation::central;
	if (get_grid_position().x == 0) { m_orientation = PortalOrientation::left; }
	if (get_grid_position().y == 0) { m_orientation = PortalOrientation::top; }
	if (get_grid_position().x > 0 && get_grid_dimensions().y > 1) { m_orientation = PortalOrientation::right; }
	if (get_grid_position().y > 0 && get_grid_dimensions().x > 1 && get_grid_dimensions().y == 1) { m_orientation = PortalOrientation::bottom; }
	if (is_already_open()) {
		m_render_state = PortalRenderState::open;
		m_attributes.set(PortalAttributes::already_open);
	}
	if (svc.data.door_is_unlocked(key_id)) { m_state.reset(PortalState::locked); }
}

std::unique_ptr<Entity> Portal::clone() const { return std::make_unique<Portal>(*this); }

void Portal::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["activate_on_contact"] = is_activate_on_contact();
	out["already_open"] = is_already_open();
	out["source_id"] = source_id;
	out["destination_id"] = destination_id;
	out["locked"] = is_locked();
	out["key_id"] = key_id;
}

void Portal::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	in["activate_on_contact"].as_bool() ? m_attributes.set(PortalAttributes::activate_on_contact) : m_attributes.reset(PortalAttributes::activate_on_contact);
	in["already_open"].as_bool() ? m_attributes.set(PortalAttributes::already_open) : m_attributes.reset(PortalAttributes::already_open);
	source_id = in["source_id"].as<int>();
	destination_id = in["destination_id"].as<int>();
	in["locked"].as_bool() ? m_state.set(PortalState::locked) : m_state.reset(PortalState::locked);
	key_id = in["key_id"].as<int>();
}

void Portal::expose() {
	Entity::expose();
	static bool activate_on_contact{is_activate_on_contact()};
	static bool already_open{is_already_open()};
	static bool locked{is_locked()};
	ImGui::InputInt("Source Room ID", &source_id);
	ImGui::InputInt("Destination Room ID", &destination_id);
	ImGui::Separator();
	ImGui::Checkbox("Activate on Contact", &activate_on_contact);
	ImGui::Checkbox("Already Open", &already_open);
	ImGui::Separator();
	ImGui::Checkbox("Locked", &locked);
	ImGui::InputInt("Key ID", &key_id);
	ImGui::Separator();
	activate_on_contact ? m_attributes.set(PortalAttributes::activate_on_contact) : m_attributes.reset(PortalAttributes::activate_on_contact);
	already_open ? m_attributes.set(PortalAttributes::already_open) : m_attributes.reset(PortalAttributes::already_open);
	locked ? m_state.set(PortalState::locked) : m_state.reset(PortalState::locked);
	if (auto const& roomdata = m_services->data.get_room_data_from_id(destination_id)) {
		if (ImGui::Button("Load Destination Room")) {
			auto const& roomstr = roomdata.value()["label"].as_string();
			auto const& regionstr = roomdata.value()["region"].as_string();
			NANI_LOG_DEBUG(m_logger, "Region: {}", regionstr);
			NANI_LOG_DEBUG(m_logger, "Room: {}", roomstr);
			m_services->events.dispatch_event("LoadFile", regionstr, roomstr);
		}
	} else {
		if (ImGui::Button("Create Destination Room")) { m_services->events.dispatch_event("NewFile", destination_id); }
	}
}

void Portal::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	Entity::update(svc, map, console, player);
	m_render_state = is_already_open() ? PortalRenderState::open : is_locked() ? PortalRenderState::locked : m_render_state;
	auto lookup = sf::IntRect({static_cast<int>(m_render_state) * constants::i_cell_resolution, map.get_style_id() * constants::i_cell_resolution * 2}, {constants::i_cell_resolution, constants::i_cell_resolution * 2});
	set_texture_rect(lookup);
	if (!map.transition.is(graphics::TransitionState::inactive)) { m_state.reset(PortalState::ready); }
	if (bounding_box.overlaps(player.get_collider().bounding_box)) {
		if (m_attributes.test(PortalAttributes::activate_on_contact)) {
			if (!m_state.test(PortalState::transitioning) && m_state.test(PortalState::ready)) { m_state.set(PortalState::activated); }
			if (is_left_or_right()) {
				if (map.transition.is(graphics::TransitionState::fading_to_black)) {
					auto towards = player.entered_from().left() ? SimpleDirection{LR::right} : SimpleDirection{LR::left};
					player.controller.direction.set_from_simple(towards);
					player.controller.autonomous_walk();
					player.walk();
				} else if (!m_state.test(PortalState::ready)) {
					player.controller.direction.set_from_simple(player.entered_from());
					player.controller.autonomous_walk();
					player.walk();
				}
			}
		} else if (player.controller.inspecting()) {
			m_state.set(PortalState::activated);
		}
	} else {
		m_state.set(PortalState::ready);
	}
	if (m_state.test(PortalState::activated)) {
		if (!console) {
			if (m_state.test(PortalState::unlocked)) {
				change_states(svc, map.room_id, map.transition);
				m_state.reset(PortalState::unlocked);
			}
		}
		if (m_state.test(PortalState::locked)) {
			if (player.has_item(key_id)) {
				m_state.reset(PortalState::locked);
				m_state.set(PortalState::unlocked);
				svc.soundboard.flags.world.set(audio::World::door_unlock);
				console = std::make_unique<gui::Console>(svc, svc.text.basic, "unlocked_door", gui::OutputType::gradual);
				console.value()->append(player.catalog.inventory.find_item(key_id)->get_title());
				console.value()->display_item(key_id);
				svc.data.unlock_door(key_id);
				svc.soundboard.flags.world.set(audio::World::door_unlock);
			} else {
				console = std::make_unique<gui::Console>(svc, svc.text.basic, "locked_door", gui::OutputType::gradual);
				m_state.reset(PortalState::activated);
			}
			return;
		}
		if (m_state.test(PortalState::unlocked)) { return; }
		change_states(svc, map.room_id, map.transition);
	}
	if (m_state.test(PortalState::transitioning)) {
		if (map.transition.is(graphics::TransitionState::black)) {
			m_state.reset(PortalState::transitioning);
			if (svc.data.exists(destination_id)) {
				svc.state_controller.next_state = destination_id;
			} else {
				svc.state_controller.next_state = source_id;
				source_id = destination_id;
				destination_id = map.room_id;
				svc.state_controller.status.set(automa::Status::out_of_bounds);
			}
			svc.state_controller.actions.set(automa::Actions::trigger);
			svc.state_controller.refresh(source_id);
		}
	}
}

void Portal::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (m_editor) { return; }
	Animatable::set_scale(constants::f_scale_vec);
	if (!m_attributes.test(PortalAttributes::activate_on_contact) && get_grid_dimensions().x * get_grid_dimensions().y == 1) {
		Animatable::set_position(get_world_position() - cam);
		win.draw(*this);
	}
}

void Portal::change_states(automa::ServiceProvider& svc, int room_id, graphics::Transition& transition) {
	if (!m_attributes.test(PortalAttributes::activate_on_contact)) {
		m_render_state = PortalRenderState::open;
		if (!m_attributes.test(PortalAttributes::already_open) && !is_large()) { svc.soundboard.flags.world.set(audio::World::door_open); }
	}
	if (m_state.test(PortalState::activated)) {
		transition.start();
		m_state.set(PortalState::transitioning);
	}
	m_state.reset(PortalState::activated);
}

} // namespace fornani
