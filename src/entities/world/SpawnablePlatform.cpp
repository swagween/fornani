#include "SpawnablePlatform.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"

namespace entity {

SpawnablePlatform::SpawnablePlatform(automa::ServiceProvider& svc, sf::Vector2<float> position, int index) : index(index), sprite(svc.assets.t_spawnable_platform, {64, 64}) {
	collider = shape::Collider({64.f, 64.f});
	collider.flags.general.set(shape::General::top_only_collision);
	gravitator = vfx::Gravitator(sf::Vector2<float>{}, sf::Color::Transparent, 0.8f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.8f, 0.8f}, 1.0f);
	gravitator.set_position(position);
	health.set_max(8.f);
	sprite.set_origin({8.f, 8.f});
	sensor.bounds.setRadius(32.f);
	sensor.bounds.setOrigin({32.f, 32.f});
	root = svc.random.random_vector_float(-16.f, 16.f);
	sprite.push_params("dormant", {0, 1, 8, -1});
	sprite.push_params("open", {1, 4, 28, -1});
	sprite.set_params("dormant");
	state = SpawnablePlatformState::dormant;
}

void SpawnablePlatform::update(automa::ServiceProvider& svc, player::Player& player, sf::Vector2<float> target) {
	gravitator.set_target_position(target + root - collider.dimensions * 0.5f);
	gravitator.update(svc);
	collider.set_position(gravitator.position());
	collider.update(svc);
	sensor.set_position(gravitator.position() + collider.dimensions * 0.5f);
	health.update();
	sprite.update(gravitator.position());
	if (state == SpawnablePlatformState::open) { player.collider.handle_collider_collision(collider); }
}

void SpawnablePlatform::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (sensor.within_bounds(proj.bounding_box)) {
		if (!proj.destruction_initiated()) {
			health.inflict(proj.get_damage());
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
			if (health.is_dead()) {
				svc.soundboard.flags.world.set(audio::World::block_toggle);
				map.effects.push_back(entity::Effect(svc, sensor.bounds.getPosition() - sf::Vector2<float>{32.f, 32.f}, {}, 0, 0));
				state = SpawnablePlatformState::open;
				sprite.set_params("open");
			}
		}
		proj.destroy(false);
	}
}

void SpawnablePlatform::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		sensor.render(win, cam);
		collider.render(win, cam);
	} else {
		sprite.render(svc, win, cam);
	}
}

} // namespace entity
