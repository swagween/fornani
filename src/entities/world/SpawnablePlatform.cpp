#include "SpawnablePlatform.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"
#include "../../utils/Math.hpp"

namespace entity {

SpawnablePlatform::SpawnablePlatform(automa::ServiceProvider& svc, sf::Vector2<float> position, int index) : index(index), sprite(svc.assets.t_spawnable_platform, {64, 64}) {
	collider = shape::Collider({64.f, 64.f});
	collider.flags.general.set(shape::General::top_only_collision);
	gravitator = vfx::Gravitator(sf::Vector2<float>{}, sf::Color::Transparent, 0.8f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.8f, 0.8f}, 1.0f);
	gravitator.set_position(position);
	health.set_max(4.f);
	sprite.set_origin({8.f, 8.f});
	sensor.bounds.setRadius(16.f);
	sensor.bounds.setOrigin({16.f, 16.f});
	sprite.push_params("dormant", {0, 1, 8, -1});
	sprite.push_params("opening", {1, 2, 8, 0});
	sprite.push_params("open", {3, 4, 28, 3});
	sprite.push_params("fading", {7, 8, 28, 0});
	sprite.push_params("closing", {15, 3, 28, 0});
	sprite.set_params("dormant");
	state = SpawnablePlatformState::dormant;
}

void SpawnablePlatform::update(automa::ServiceProvider& svc, player::Player& player, sf::Vector2<float> target) {
	gravitator.set_target_position(target - collider.dimensions * 0.5f);
	gravitator.update(svc);
	collider.update(svc);
	collider.set_position(gravitator.position());
	if (collidable()) { player.collider.handle_collider_collision(collider, false, true); }
	collider.physics.previous_position = gravitator.position();
	sensor.set_position(gravitator.position() + collider.dimensions * 0.5f - sf::Vector2<float>{0.f, 4.f});
	health.update();
	sprite.update(util::round_to_even(gravitator.position() - sf::Vector2<float>{-4.f, 2.f}));
	state_function = state_function();
}

void SpawnablePlatform::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (sensor.within_bounds(proj.bounding_box) && !health.is_dead() && state == SpawnablePlatformState::dormant) {
		if (!proj.destruction_initiated()) {
			health.inflict(proj.get_damage());
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
			if (health.is_dead()) {
				svc.soundboard.flags.world.set(audio::World::block_toggle);
				map.effects.push_back(entity::Effect(svc, sensor.bounds.getPosition() - sf::Vector2<float>{32.f, 32.f}, {}, 0, 0));
				state = SpawnablePlatformState::opening;
			}
		}
		proj.destroy(false);
	}
}

void SpawnablePlatform::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
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
		health.refill();
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

} // namespace entity
