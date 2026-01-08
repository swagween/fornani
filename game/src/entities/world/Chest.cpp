
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Chest.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::entity {

Chest::Chest(automa::ServiceProvider& svc, world::Map& map, int id, ChestType type, int modifier) : Animatable(svc, "chests", {16, 16}), m_id(id), m_type{type}, m_content_modifier{modifier}, m_collider{map, 14.f} {
	get_collider().physics.elasticity = 0.4f;
	get_collider().physics.set_friction_componentwise({0.99f, 0.998f});
	get_collider().physics.gravity = 10.f;
	get_collider().set_exclusion_target(shape::ColliderTrait::player);
	get_collider().set_exclusion_target(shape::ColliderTrait::npc);

	Animatable::set_parameters(m_animations.unopened);

	if (svc.data.chest_is_open(id) && id != -1) { state.set(ChestState::open); }
	if (type == ChestType::item) { m_item_label = svc.data.item_label_from_id(modifier); }
}

void Chest::update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player) {

	if (m_attributes.test(ChestAttributes::mythic)) {
		set_channel(1);
		if (!m_sparkler) {
			m_sparkler = vfx::Sparkler(svc, get_f_dimensions() * 2.f, colors::ui_white, "chest");
			m_sparkler->activate();
		}
	}
	if (m_sparkler) {
		m_sparkler->set_position(get_collider().physics.position - get_f_dimensions());
		m_sparkler->update(svc);
	}

	Animatable::tick();
	if (!state.test(ChestState::open)) {
		if (svc.ticker.every_x_ticks(1200)) { Animatable::set_parameters(m_animations.shine); }
		if (animation.complete() && !state.test(ChestState::activated)) { Animatable::set_parameters(m_animations.unopened); }
	} else {
		Animatable::set_parameters(m_animations.opened);
	}

	if (get_collider().collided() && std::abs(get_collider().physics.apparent_velocity().y) > 0.05f) { svc.soundboard.flags.world.set(audio::World::clink); }
	map.handle_cell_collision(get_collider());
	for (auto& button : map.switch_buttons) { get_collider().handle_collision(button->collider.bounding_box); }
	get_collider().physics.acceleration = {};
	state.reset(ChestState::activated);

	if (get_collider().collides_with(player.get_collider().bounding_box)) {
		if (player.controller.inspecting()) {
			state.set(ChestState::activated);
			if (!state.test(ChestState::open)) {
				if (m_sparkler) { m_sparkler->deactivate(); }
				svc.soundboard.flags.world.set(audio::World::chest);
				state.set(ChestState::open);
				Animatable::set_parameters(m_animations.opened);
				if (m_id != -1) { svc.data.open_chest(m_id); }
				auto fmodifier = static_cast<float>(m_content_modifier);
				auto range_modifier = std::max(6, static_cast<int>(m_content_modifier / 8.f));
				if (m_type == ChestType::gun) { svc.events.dispatch_event("AcquireGun", m_content_modifier); }
				if (m_type == ChestType::orbs) { map.active_loot.push_back(item::Loot(svc, map, player, {range_modifier, range_modifier * 2}, fmodifier, get_collider().get_global_center(), 100, true, map.get_special_drop_id())); }
				if (m_type == ChestType::item) { svc.events.dispatch_event("AcquireItem", m_content_modifier); }
			} else {
				console = std::make_unique<gui::Console>(svc, svc.text.basic, "open_chest", gui::OutputType::instant);
			}
		}
		if (console && state.test(ChestState::open) && m_type != ChestType::orbs) {
			svc.camera_controller.free();
		} else {
			svc.camera_controller.constrain();
		}
	}
}

void Chest::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto sprite_position = get_collider().physics.position - cam + sf::Vector2f{-2.f, -4.f} - get_collider().get_local_center();
	Drawable::set_position(sprite_position);
	if (m_sparkler) { m_sparkler->render(win, cam); }
	win.draw(*this);
}

void Chest::set_position(sf::Vector2f pos) { get_collider().set_position(pos); }

void Chest::set_position_from_scaled(sf::Vector2f scaled_pos) { get_collider().set_position(scaled_pos * constants::f_cell_size + sf::Vector2f{16.f, 18.f}); }

} // namespace fornani::entity
