#include "SwitchButton.hpp"
#include <cmath>
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../particle/Effect.hpp"
#include "Breakable.hpp"
#include <algorithm>

namespace world {

SwitchButton::SwitchButton(automa::ServiceProvider& svc, sf::Vector2<float> position, int id, int type) : sprite(svc.assets.t_switches, {32, 16}), id(id) {
	collider = shape::Collider({32.f, 16.f});
	collider.physics.position = position;
	collider.physics.set_global_friction(0.99f);
	collider.physics.elasticity = 0.3f;
	collider.stats.GRAV = 1.4f;
	collider.sync_components();
	collider.collision_depths = util::CollisionDepth();
	collider.flags.general.set(shape::General::soft);
	sensor.dimensions = {28.f, 2.f};
	sprite.push_params("neutral", {0, 1, 28, -1});
	sprite.push_params("shine", {1, 4, 28, 0});
	sprite.push_params("squished", {5, 1, 28, 0});
	sprite.push_params("pressed", {6, 1, 28, 0});
	sprite.push_params("rising", {7, 1, 28, 0});
	sprite.set_params("neutral");
}

void SwitchButton::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	released_cooldown.update();
	pressed_cooldown.update();
	if (collider.collision_depths) { collider.collision_depths.value().reset(); }
	sensor.set_position(collider.physics.position + sf::Vector2<float>{2.f, 0.f});
	sprite.update(collider.physics.position + sprite_offset, static_cast<int>(type), static_cast<int>(state));

	for (auto& block : map.switch_blocks) {
		if (block.get_id() == id) { pressed() ? block.turn_off() : block.turn_on(); }
	}

	if (state == SwitchButtonState::unpressed && !triggers.test(SwitchTriggers::released) && !pressed_cooldown.running()) {
		sprite.set_params("neutral");
		collider.dimensions.y = 16.f;
		collider.physics.position.y -= 10.f;
		triggers.set(SwitchTriggers::released);
		triggers.reset(SwitchTriggers::pressed);
	}
	if (state == SwitchButtonState::pressed && !triggers.test(SwitchTriggers::pressed) && !released_cooldown.running()) {
		sprite.set_params("pressed");
		collider.dimensions.y = 6.f;
		collider.physics.position.y += 10.f;
		triggers.set(SwitchTriggers::pressed);
		triggers.reset(SwitchTriggers::released);
	}

	if (player.collider.predictive_horizontal.overlaps(collider.bounding_box)) { collider.physics.velocity.x = player.collider.physics.acceleration.x * 0.5f; }
	collider.update(svc);
	
	collider.handle_collider_collision(player.collider.bounding_box);
	for (auto& breakable : map.breakables) { collider.handle_collider_collision(breakable.get_bounding_box()); }
	for (auto& platform : map.platforms) {
		collider.handle_collider_collision(platform.bounding_box);
		if (platform.jumpbox.overlaps(sensor)) {
			state = SwitchButtonState::pressed;
			pressed_cooldown.start();
		}
	}
	for (auto& chest : map.chests) {
		if (chest.get_jumpbox().overlaps(sensor)) {
			state = SwitchButtonState::pressed;
			pressed_cooldown.start();
		}
	}
	if (player.collider.jumpbox.overlaps(sensor)) {
		state = SwitchButtonState::pressed;
		pressed_cooldown.start();
	} else if(!pressed_cooldown.running()) {
		state = SwitchButtonState::unpressed;
		released_cooldown.start();
	}

	collider.detect_map_collision(map);
	handle_collision(player.collider);
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};
	if (collider.collision_depths) {
		if (collider.collision_depths.value().vertical_squish()) { state = SwitchButtonState::pressed; }
		collider.collision_depths.value().update();
	}
}

void SwitchButton::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider.bounding_box); }

void SwitchButton::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
		return;
	}
	sprite.render(svc, win, cam);
}

void SwitchButton::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.stats.transcendent) { return; }
	if (proj.bounding_box.overlaps(collider.bounding_box)) {
		state = SwitchButtonState::pressed;
		pressed_cooldown.start();
		if (!proj.destruction_initiated()) {
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
		}
		proj.destroy(false);
	}
}

} // namespace world
