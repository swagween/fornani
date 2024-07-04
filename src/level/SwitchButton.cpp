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
	collider.physics.position.y += 16.f;
	collider.physics.set_global_friction(0.99f);
	collider.physics.elasticity = 0.3f;
	collider.stats.GRAV = 1.4f;
	collider.sync_components();
	collider.collision_depths = util::CollisionDepth();
	collider.flags.general.set(shape::General::soft);
	sensor.dimensions = {32.f, 16.f};
	sensor.set_position(position);
	sprite.push_params("neutral", {0, 1, 28, -1});
	sprite.push_params("shine", {1, 4, 28, 0});
	sprite.push_params("squished", {5, 1, 28, 0, false, true});
	sprite.push_params("pressed", {5, 2, 28, 0, true});
	sprite.push_params("rising", {7, 1, 28, 0});
	sprite.set_params("neutral");
}

void SwitchButton::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	released_cooldown.update();
	pressed_cooldown.update();
	if (collider.collision_depths) { collider.collision_depths.value().reset(); }
	sprite.update(collider.physics.position + sprite_offset, static_cast<int>(type), static_cast<int>(state));

	for (auto& block : map.switch_blocks) {
		if (block.get_id() == id) { pressed() ? block.turn_off() : block.turn_on(); }
	}
	if (player.collider.predictive_horizontal.overlaps(collider.bounding_box)) { collider.physics.velocity.x = player.collider.physics.acceleration.x * 0.5f; }
	collider.update(svc);
	
	collider.handle_collider_collision(player.collider.bounding_box);
	for (auto& breakable : map.breakables) { collider.handle_collider_collision(breakable.get_bounding_box()); }
	for (auto& platform : map.platforms) {
		if (platform.bounding_box.overlaps(sensor)) {
			state = SwitchButtonState::pressed;
		}
	}
	for (auto& chest : map.chests) {
		if (chest.get_jumpbox().overlaps(sensor)) {
			state = SwitchButtonState::pressed;
		}
	}
	if (player.collider.jumpbox.overlaps(sensor)) {
		state = SwitchButtonState::pressed;
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

	state_function = state_function();
}

void SwitchButton::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider.bounding_box); }

void SwitchButton::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
		sensorbox.setPosition(sensor.position - cam);
		sensorbox.setSize(sensor.dimensions);
		sensorbox.setFillColor(sf::Color::Transparent);
		sensorbox.setOutlineThickness(-4);
		pressed() ? sensorbox.setOutlineColor(svc.styles.colors.dark_fucshia) : sensorbox.setOutlineColor(svc.styles.colors.mythic_green);
		win.draw(sensorbox);
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

fsm::StateFunction SwitchButton::update_unpressed() {
	if (sprite.just_started()) {
		std::cout << "Unpressed started.\n";
		collider.dimensions.y = 16.f;
		sensor.set_position(collider.physics.position);
	}
	if (change_state(SwitchButtonState::pressed, "squished")) { return SWITCH_BIND(update_squished); }
	if (change_state(SwitchButtonState::shining, "shine")) { return SWITCH_BIND(update_shining); }
	state = SwitchButtonState::unpressed;
	return SWITCH_BIND(update_unpressed);
}

fsm::StateFunction SwitchButton::update_shining() {
	if (sprite.just_started()) {
		collider.dimensions.y = 16.f;
		//collider.physics.position.y -= -(16.f - collider.physics.position.y);
		sensor.set_position(collider.physics.position);
	}
	if (change_state(SwitchButtonState::unpressed, "neutral")) { return SWITCH_BIND(update_unpressed); }
	state = SwitchButtonState::shining;
	return SWITCH_BIND(update_shining);
}

fsm::StateFunction SwitchButton::update_squished() {
	if (sprite.just_started()) {
		collider.dimensions.y = 12.f;
		collider.physics.position.y += 4.f;
		sensor.set_position(collider.physics.position + sf::Vector2<float>{0.f, -4.f});
	}
	if (sprite.complete()) {
		if (change_state(SwitchButtonState::pressed, "pressed")) { return SWITCH_BIND(update_pressed); }
	}
	state = SwitchButtonState::squishing;
	return SWITCH_BIND(update_squished);
}

fsm::StateFunction SwitchButton::update_pressed() {
	if (sprite.just_started()) {
		collider.dimensions.y = 6.f;
		collider.physics.position.y += 6.f;
		sensor.set_position(collider.physics.position + sf::Vector2<float>{0.f, -10.f});
	}
	if (change_state(SwitchButtonState::unpressed, "rising")) { return SWITCH_BIND(update_rising); }
	state = SwitchButtonState::pressed;
	return SWITCH_BIND(update_pressed);
}

fsm::StateFunction SwitchButton::update_rising() {
	if (sprite.just_started()) {
		collider.dimensions.y = 12.f;
		//collider.physics.position.y -= -(12.f - collider.physics.position.y);
		//sensor.set_position(collider.physics.position + sf::Vector2<float>{0.f, -(12.f - collider.dimensions.y)});
	}
	if (change_state(SwitchButtonState::unpressed, "neutral")) { return SWITCH_BIND(update_unpressed); }
	state = SwitchButtonState::rising;
	return SWITCH_BIND(update_rising);
}

bool SwitchButton::change_state(SwitchButtonState next, std::string_view tag) {
	if (state == next) {
		sprite.set_params(tag, true);
		std::cout << "State changed.\n";
		return true;
	}
	return false;
}

} // namespace world
