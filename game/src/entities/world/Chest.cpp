
#include "fornani/entities/world/Chest.hpp"
#include <fornani/gui/console/Console.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::entity {

Chest::Chest(automa::ServiceProvider& svc, int id, ChestType type, int modifier) : Animatable(svc, "chests", {16, 16}), m_id(id), m_type{type}, m_content_modifier{modifier}, collider{14.f} {
	collider.physics.elasticity = 0.4f;
	collider.physics.set_global_friction(0.998f);
	collider.physics.gravity = 10.f;

	Animatable::set_parameters(m_animations.unopened);

	if (svc.data.chest_is_open(id)) { state.set(ChestState::open); }
	if (type == ChestType::item) { m_item_label = svc.data.item_label_from_id(modifier); }
}

void Chest::update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player) {

	Animatable::tick();
	if (!state.test(ChestState::open)) {
		if (svc.ticker.every_x_ticks(1200)) { Animatable::set_parameters(m_animations.shine); }
		if (animation.complete() && !state.test(ChestState::activated)) { Animatable::set_parameters(m_animations.unopened); }
	} else {
		Animatable::set_parameters(m_animations.opened);
	}

	collider.update(svc);
	collider.handle_map_collision(map);
	if (collider.collided() && std::abs(collider.physics.apparent_velocity().y) > 0.05f) { svc.soundboard.flags.world.set(audio::World::clink); }
	map.handle_cell_collision(collider);
	map.handle_breakable_collision(collider);
	for (auto& pushable : map.pushables) { collider.handle_collision(pushable.collider.bounding_box); }
	for (auto& platform : map.platforms) { collider.handle_collision(platform.bounding_box); }
	for (auto& destructible : map.destructibles) {
		if (!destructible.ignore_updates()) { collider.handle_collision(destructible.get_bounding_box()); }
	}
	for (auto& button : map.switch_buttons) { collider.handle_collision(button->collider.bounding_box); }
	for (auto& block : map.switch_blocks) {
		if (block.on()) { collider.handle_collision(block.collider.bounding_box); }
	}
	collider.physics.acceleration = {};
	state.reset(ChestState::activated);

	if (collider.collides_with(player.collider.bounding_box)) {
		if (player.controller.inspecting()) {
			state.set(ChestState::activated);
			if (!state.test(ChestState::open)) {
				svc.soundboard.flags.world.set(audio::World::chest);
				state.set(ChestState::open);
				Animatable::set_parameters(m_animations.opened);
				svc.data.open_chest(m_id);
				if (m_type == ChestType::gun) { svc.events.dispatch_event("AcquireGun", m_content_modifier); }
				if (m_type == ChestType::orbs) { map.active_loot.push_back(item::Loot(svc, {6, 12}, static_cast<float>(m_content_modifier), collider.get_global_center(), 100)); }
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
	auto sprite_position = collider.physics.position - cam + sf::Vector2f{-2.f, -4.f} - collider.get_local_center();
	Drawable::set_position(sprite_position);
	win.draw(*this);
}

void Chest::set_position(sf::Vector2f pos) { collider.set_position(pos); }

void Chest::set_position_from_scaled(sf::Vector2f scaled_pos) { collider.set_position(scaled_pos * constants::f_cell_size + sf::Vector2f{0.f, 18.f}); }

} // namespace fornani::entity
