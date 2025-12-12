#include "fornani/entities/world/SpawnablePlatform.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::entity {

SpawnablePlatform::SpawnablePlatform(automa::ServiceProvider& svc, sf::Vector2f position, int index) : index(index), sprite(svc.assets.get_texture("spawnable_platform"), {32, 32}), m_health{1.f} {
	collider = shape::Collider({64.f, 64.f});
	collider.set_top_only();
	gravitator = vfx::Gravitator(sf::Vector2f{}, sf::Color::Transparent, 0.8f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2f{0.8f, 0.8f}, 1.0f);
	gravitator.set_position(position);
	sprite.set_origin({2.f, 4.f});
	sensor.bounds.setRadius(24.f);
	sensor.bounds.setOrigin({24.f, 24.f});
	sprite.push_params("dormant", {0, 1, 8, -1});
	sprite.push_params("opening", {1, 2, 8, 0});
	sprite.push_params("open", {3, 4, 28, 3});
	sprite.push_params("fading", {7, 8, 28, 0});
	sprite.push_params("closing", {15, 3, 28, 0});
	sprite.set_params("dormant");
	state = SpawnablePlatformState::dormant;
}

void SpawnablePlatform::update(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f target) {
	gravitator.set_target_position(target - collider.dimensions * 0.5f);
	gravitator.update(svc);
	collider.update(svc);
	collider.set_position(gravitator.position());
	if (collidable()) { player.collider.handle_collider_collision(collider, false, true); }
	collider.physics.previous_position = gravitator.position();
	sensor.set_position(gravitator.position() + collider.dimensions * 0.5f - sf::Vector2f{0.f, 16.f});
	m_health.update();
	sprite.update(util::round_to_even(gravitator.position() - sf::Vector2f{-4.f, 2.f}));
	state_function = state_function();
}

void SpawnablePlatform::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (sensor.within_bounds(proj.get_collider()) && !m_health.is_dead() && state == SpawnablePlatformState::dormant) {
		if (!proj.destruction_initiated()) {
			m_health.inflict(proj.get_damage());
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
			if (m_health.is_dead()) {
				svc.soundboard.flags.world.set(audio::World::block_toggle);
				map.effects.push_back(entity::Effect(svc, "small_explosion", sensor.bounds.getPosition()));
				state = SpawnablePlatformState::opening;
			}
		}
		proj.destroy(false);
	}
}

void SpawnablePlatform::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
		sensor.render(win, cam);
	} else {
		sprite.render(svc, win, cam);
	}
}

fsm::StateFunction SpawnablePlatform::update_open() {
	if (sprite.complete()) {
		state = SpawnablePlatformState::fading;
		sprite.set_params("fading", true);
		return SPAWNABLE_PLAT_BIND(update_fading);
	}
	return SPAWNABLE_PLAT_BIND(update_open);
}

fsm::StateFunction SpawnablePlatform::update_opening() {
	if (sprite.complete()) {
		state = SpawnablePlatformState::open;
		sprite.set_params("open", true);
		return SPAWNABLE_PLAT_BIND(update_open);
	}
	return SPAWNABLE_PLAT_BIND(update_opening);
}

fsm::StateFunction SpawnablePlatform::update_fading() {
	if (sprite.complete()) {
		state = SpawnablePlatformState::closing;
		sprite.set_params("closing", true);
		return SPAWNABLE_PLAT_BIND(update_closing);
	}
	return SPAWNABLE_PLAT_BIND(update_fading);
}

fsm::StateFunction SpawnablePlatform::update_closing() {
	if (sprite.complete()) {
		m_health.refill();
		state = SpawnablePlatformState::dormant;
		sprite.set_params("dormant", true);
		return SPAWNABLE_PLAT_BIND(update_dormant);
	}
	return SPAWNABLE_PLAT_BIND(update_closing);
}

fsm::StateFunction SpawnablePlatform::update_dormant() {
	if (change_state(SpawnablePlatformState::opening, "opening")) { return SPAWNABLE_PLAT_BIND(update_opening); }
	return SPAWNABLE_PLAT_BIND(update_dormant);
}

bool SpawnablePlatform::change_state(SpawnablePlatformState next, std::string_view tag) {
	if (state == next) {
		sprite.set_params(tag, true);
		return true;
	}
	return false;
}

} // namespace fornani::entity
