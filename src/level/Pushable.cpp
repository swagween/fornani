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
	start_position = position;
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
	energy = std::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = svc.random.random_vector_float(-energy, energy); }
	weakened.update();
	if (weakened.is_complete()) { hit_count.start(); }
	if (hit_count.get_count() > 3 || map.off_the_bottom(collider.physics.position)) {
		reset(svc, map);
		svc.soundboard.flags.world.set(audio::World::small_crash);
		hit_count.start();
	}
	if (player.collider.wallslider.overlaps(collider.bounding_box) && player.pushing()) {
		if (player.controller.moving_left() && player.collider.physics.position.x > collider.physics.position.x) { collider.physics.acceleration.x = -speed / mass; }
		if (player.controller.moving_right() && player.collider.physics.position.x < collider.physics.position.x) { collider.physics.acceleration.x = speed / mass; }
		state.set(PushableState::moved);
	}
	player.collider.handle_collider_collision(collider.bounding_box);
	collider.handle_collider_collision(player.collider.bounding_box);
	collider.physics.position += forced_momentum;
	if (!collider.has_jump_collision()) { forced_momentum = {}; }
	if (collider.has_left_wallslide_collision() || collider.has_right_wallslide_collision() || collider.flags.external_state.test(shape::ExternalState::vert_world_collision) || collider.world_grounded()) { forced_momentum = {}; }
	collider.update(svc);
	collider.detect_map_collision(map);
	for (auto& other : map.pushables) {
		if (&other == this) { continue; }
		if (other.collider.wallslider.overlaps(collider.bounding_box)) {
			if (collider.pushes(other.collider)) { other.collider.physics.velocity.x = collider.physics.velocity.x * 2.f; }
		}
		collider.handle_collider_collision(other.collider.bounding_box);
	}
	for (auto& spike : map.spikes) { collider.handle_collider_collision(spike.get_bounding_box()); }
	for (auto& breakable : map.breakables) { collider.handle_collider_collision(breakable.get_bounding_box()); }
	for (auto& platform : map.platforms) { collider.handle_collider_collision(platform.bounding_box); }
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
	sprite.setPosition(snap - cam + random_offset);
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		win.draw(sprite);
	}
}

void Pushable::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.stats.transcendent) { return; }
	if (proj.bounding_box.overlaps(collider.bounding_box)) {
		hit_count.update();
		weakened.start();
		if (!proj.destruction_initiated()) {
			energy = hit_energy;
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
		}
		proj.destroy(false);
	}
}

void Pushable::reset(automa::ServiceProvider& svc, world::Map& map) {
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) { map.effects.push_back(entity::Effect(svc, collider.physics.position + sf::Vector2<float>{32.f * i, 32.f * j}, {}, 0, 0)); }
	}
	collider.physics.position = start_position;
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) { map.effects.push_back(entity::Effect(svc, collider.physics.position + sf::Vector2<float>{32.f * i, 32.f * j}, {}, 0, 0)); }
	}
}

} // namespace world
