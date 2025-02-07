#include "fornani/level/Pushable.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/particle/Effect.hpp"
#include <algorithm>
#include <cmath>

namespace world {

Pushable::Pushable(automa::ServiceProvider& svc, sf::Vector2<float> position, int style, int size) : style(style), size(size), sprite{svc.assets.t_pushables} {
	collider = shape::Collider({svc.constants.cell_size * static_cast<float>(size) - 4.f, svc.constants.cell_size * static_cast<float>(size) - 1.f});
	collider.physics.position = position;
	start_position = position;
	collider.physics.set_constant_friction({0.95f, 0.98f});
	collider.physics.air_friction.x = 0.2f;
	collider.stats.GRAV = 18.0f;
	mass = static_cast<float>(size);
	collider.sync_components();
	auto lock = collider.snap_to_grid(static_cast<float>(size));
	collider.physics.position = lock;
	start_box = collider.bounding_box;
	sf::IntRect lookup = size == 1 ? sf::IntRect{{style * 2 * svc.constants.i_cell_size, 0}, svc.constants.i_cell_vec} : sf::IntRect{{style * 2 * svc.constants.i_cell_size, svc.constants.i_cell_size}, 2 * svc.constants.i_cell_vec};
	sprite.setTextureRect(lookup);
}

void Pushable::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	energy = std::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = svc.random.random_vector_float(-energy, energy); }
	weakened.update();
	if (weakened.is_complete()) { hit_count.start(); }
	player.on_crush(map);
	for (auto& enemy : map.enemy_catalog.enemies) { enemy->on_crush(map); }

	//reset position if it's far away, and if the player isn't overlapping the start position
	if (hit_count.get_count() > 2 || map.off_the_bottom(collider.physics.position)) {
		bool can_respawn = true;
		if (player.collider.bounding_box.overlaps(start_box)) { can_respawn = false; }
		for (auto& p : map.pushables) {
			if (p.get_bounding_box().overlaps(start_box) && &p != this) { can_respawn = false; }
		}
		if (can_respawn) {
			reset(svc, map);
			svc.soundboard.flags.world.set(audio::World::small_crash);
		}
		hit_count.start();
	}

	//player pushes block
	if (player.collider.wallslider.overlaps(collider.bounding_box) && player.pushing()) {
		if (player.controller.moving_left() && player.collider.physics.position.x > collider.physics.position.x) { collider.physics.acceleration.x = -speed / mass; }
		if (player.controller.moving_right() && player.collider.physics.position.x < collider.physics.position.x) { collider.physics.acceleration.x = speed / mass; }
		if (abs(collider.physics.acceleration.x) > 0.f) { svc.soundboard.flags.world.set(audio::World::pushable); }
		state.set(PushableState::moved);
		state.set(PushableState::moving);
	}

	//debug
	if(state.test(PushableState::moved)) {
		/*if (svc.ticker.every_x_ticks(400)) {
			std::cout << "X: " << collider.physics.position.x << "\n";
			std::cout << "Y: " << collider.physics.position.y << "\n";
			std::cout << "Snap X: " << snap.x << "\n";
			std::cout << "Snap Y: " << snap.y << "\n";
			std::cout << "---\n";
		}*/
	}

	player.collider.handle_collider_collision(collider);
	for (auto& enemy : map.enemy_catalog.enemies) {
		if (enemy->is_transcendent()) { continue; }
		enemy->get_collider().handle_collider_collision(collider);
		if (size == 1) {
			collider.handle_collider_collision(enemy->get_collider().bounding_box);
			collider.handle_collider_collision(enemy->get_secondary_collider().bounding_box);
		}
	}
	if (size == 1) { collider.handle_collider_collision(player.collider.bounding_box); } // big ones should crush the player
	if (abs(collider.physics.forced_momentum.x) > 0.1f || abs(collider.physics.forced_momentum.y) > 0.1f) { set_moving(); }
	collider.physics.impart_momentum();
	if (!collider.has_jump_collision()) { collider.physics.forced_momentum = {}; }
	if (collider.has_left_wallslide_collision() || collider.has_right_wallslide_collision() || collider.flags.external_state.test(shape::ExternalState::vert_world_collision) || collider.world_grounded()) { collider.physics.forced_momentum = {}; }
	collider.update(svc);
	collider.detect_map_collision(map);
	for (auto& other : map.pushables) {
		if (&other == this) { continue; }
		if (other.collider.wallslider.overlaps(collider.bounding_box)) {
			if (collider.pushes(other.collider)) {
				other.collider.physics.velocity.x = collider.physics.velocity.x * 2.f;
			}
		}
		collider.handle_collider_collision(other.collider.bounding_box);
	}
	//for (auto& spike : map.spikes) { collider.handle_collider_collision(spike.get_bounding_box()); }
	for (auto& breakable : map.breakables) { collider.handle_collider_collision(breakable.get_bounding_box()); }
	for (auto& block : map.switch_blocks) {
		if (block.on()) { collider.handle_collider_collision(block.get_bounding_box()); }
	}
	// pushable should only be moved by a platform if it's on top of one
	for (auto& platform : map.platforms) {
		if (platform.bounding_box.overlaps(collider.jumpbox)) { collider.handle_collider_collision(platform.bounding_box); }
	}
	if (collider.flags.state.test(shape::State::just_landed)) {
		map.effects.push_back(entity::Effect(svc, {collider.physics.position.x + 32.f * (size / 2.f), collider.physics.position.y + (size - 1) * 32.f}, {}, 0, 10));
		svc.soundboard.flags.world.set(audio::World::thud);
	}
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};
}

void Pushable::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider); }

void Pushable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	snap = collider.snap_to_grid(1, 4.f, 2.f);
	if (abs(random_offset.x) > 0.f || abs(random_offset.y) > 0.f) { snap = collider.physics.position; } // don't snap if shaking
	if (abs(collider.physics.velocity.x) > 0.1f || abs(collider.physics.velocity.y) > 0.1f) { set_moving(); }
	if (is_moving()) { snap = collider.physics.position; } // don't snap if moving
	state.reset(PushableState::moving); // we only use this flag for rendering
	sprite.setPosition(snap - cam + random_offset - sprite_offset);
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
		sf::RectangleShape box{};
		box.setSize(start_box.get_dimensions());
		box.setFillColor(sf::Color::Transparent);
		box.setOutlineColor(sf::Color::Green);
		box.setOutlineThickness(-1);
		box.setPosition(start_box.get_position() - cam);
		win.draw(box);
	} else {
		win.draw(sprite);
	}
}

void Pushable::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (proj.get_bounding_box().overlaps(collider.bounding_box)) {
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
	auto index = size == 1 ? 0 : 1;
	auto offset = size == 1 ? sf::Vector2<float>{} : sf::Vector2<float>{5.f, 5.f};
	map.effects.push_back(entity::Effect(svc, collider.physics.position + offset, {}, 0, index));
	collider.physics.position = start_position;
	map.effects.push_back(entity::Effect(svc, collider.physics.position + offset, {}, 0, index));
}

} // namespace world
