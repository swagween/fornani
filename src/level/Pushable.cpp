#include "Pushable.hpp"
#include <cmath>
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../particle/Effect.hpp"
#include <algorithm>

namespace world {

Pushable::Pushable(automa::ServiceProvider& svc, sf::Vector2<float> position, int style, int size) : style(style), size(size) {
	collider = shape::Collider({svc.constants.cell_size * 0.99f * static_cast<float>(size), svc.constants.cell_size * 0.99f * static_cast<float>(size)});
	collider.physics.position = position;
	collider.physics.set_constant_friction({0.95f, 0.98f});
	collider.physics.air_friction.x = 0.2f;
	collider.stats.GRAV = 18.0f;
	mass = static_cast<float>(size);
	collider.sync_components();
	auto snap = collider.snap_to_grid(static_cast<float>(size));
	collider.physics.position = snap;
	sprite.setTexture(svc.assets.t_pushables);
	sf::IntRect lookup = size == 1 ? sf::IntRect{{style * 2 * svc.constants.i_cell_size, 0}, svc.constants.i_cell_vec} : sf::IntRect{{style * 2 * svc.constants.i_cell_size, svc.constants.i_cell_size}, 2 * svc.constants.i_cell_vec};
	sprite.setTextureRect(lookup);
}

void Pushable::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (player.collider.wallslider.overlaps(collider.bounding_box) && player.pushing()) {
		if (player.controller.moving_left() && player.collider.physics.position.x > collider.physics.position.x) { collider.physics.acceleration.x = -speed / mass; }
		if (player.controller.moving_right() && player.collider.physics.position.x < collider.physics.position.x) { collider.physics.acceleration.x = speed / mass; }
		state.set(PushableState::moved);
	}
	player.collider.handle_collider_collision(collider.bounding_box);
	collider.handle_collider_collision(player.collider.bounding_box);
	collider.update(svc);
	collider.detect_map_collision(map);
	for (auto& other : map.pushables) {
		if (&other == this) { continue; }
		collider.handle_collider_collision(other.collider.bounding_box);
		//if (other.collider.wallslider.overlaps(collider.bounding_box)) { other.collider.physics.velocity.x = collider.physics.velocity.x * 0.5f; }
	}
	for (auto& breakable : map.breakables) { collider.handle_collider_collision(breakable.get_bounding_box()); }
	if (collider.flags.state.test(shape::State::just_landed)) {
		map.effects.push_back(entity::Effect(svc, {collider.physics.position.x + 32.f * (size / 2.f), collider.physics.position.y + (size - 1) * 32.f}, {}, 0, 10));
		svc.soundboard.flags.world.set(audio::World::thud);
	}
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};
}

void Pushable::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider.bounding_box); }

void Pushable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	auto snap = collider.snap_to_grid(static_cast<float>(size), 2.f, 2.f);
	sprite.setPosition(snap - cam);
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		win.draw(sprite);
	}
}

void Pushable::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.stats.transcendent) { return; }
	if (proj.bounding_box.overlaps(collider.bounding_box)) {
		if (!proj.destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, proj.physics.position, {}, 0, 6));
			svc.soundboard.flags.world.set(audio::World::hard_hit);
		}
		proj.destroy(false);
	}
}

} // namespace world
