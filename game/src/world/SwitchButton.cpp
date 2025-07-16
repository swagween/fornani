
#include "fornani/world/SwitchButton.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Breakable.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

SwitchButton::SwitchButton(automa::ServiceProvider& svc, sf::Vector2f position, int id, int type, Map& map) : sprite(svc.assets.get_texture("switch_buttons"), {16, 8}), id(id), type(static_cast<SwitchType>(type)) {
	collider = shape::Collider({32.f, 14.f});
	collider.physics.position = position;
	collider.physics.position.y += 18.f;
	collider.physics.set_global_friction(0.99f);
	collider.physics.elasticity = 0.3f;
	collider.stats.GRAV = 1.4f;
	collider.sync_components();
	collider.collision_depths = util::CollisionDepth();
	collider.flags.general.set(shape::General::soft);
	sensor.set_dimensions({28.f, 16.f});
	sensor.set_position(position);
	sprite.push_params("neutral", {0, 1, 28, -1});
	sprite.push_params("shine", {1, 4, 28, 0});
	sprite.push_params("squished", {5, 1, 28, 0, false, true});
	sprite.push_params("pressed", {6, 1, 28, 0, true});
	sprite.push_params("rising", {7, 1, 28, 0, false, true});
	sprite.set_params("neutral");
	if (svc.data.switch_is_activated(id)) {
		state = SwitchButtonState::pressed;
		state_function = std::bind(&SwitchButton::update_pressed, this);
		collider.dimensions.y = 4.f;
		collider.physics.position.y += 6.f;
		sprite.set_params("pressed", true);
		for (auto& block : map.switch_blocks) {
			if (block.get_id() == id && pressed()) { block.turn_off(); }
		}
	}
}

void SwitchButton::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (collider.collision_depths) { collider.collision_depths.value().reset(); }
	sprite.update(collider.physics.position, static_cast<int>(type), static_cast<int>(state));

	for (auto& block : map.switch_blocks) {
		if (block.get_id() == id) { pressed() ? block.turn_off() : block.turn_on(); }
	}

	// type-specific stuff
	if (type == SwitchType::movable) {
		if (player.collider.predictive_horizontal.overlaps(collider.bounding_box)) { collider.physics.velocity.x = player.collider.physics.acceleration.x * 0.5f; }
		collider.update(svc);
		collider.handle_collider_collision(player.collider.bounding_box);
	}

	// press permanent switches forever
	if (type == SwitchType::permanent && pressed()) { svc.data.activate_switch(id); }

	// assume unpressed, then check everything for a press
	if (type != SwitchType::permanent) { state = SwitchButtonState::unpressed; }
	for (auto& breakable : map.breakables) { collider.handle_collider_collision(breakable.get_bounding_box()); }
	for (auto& platform : map.platforms) {
		if (platform.bounding_box.overlaps(sensor)) { state = SwitchButtonState::pressed; }
	}
	for (auto& chest : map.chests) {
		if (chest.get_jumpbox().overlaps(sensor)) { state = SwitchButtonState::pressed; }
	}
	for (auto& pushable : map.pushables) {
		if (pushable.collider.jumpbox.overlaps(sensor)) { state = SwitchButtonState::pressed; }
	}
	if (player.collider.jumpbox.overlaps(sensor)) { state = SwitchButtonState::pressed; }

	collider.detect_map_collision(map);
	handle_collision(player.collider);
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};
	if (collider.collision_depths) {
		if (collider.collision_depths.value().vertical_squish()) { state = SwitchButtonState::pressed; }
		collider.collision_depths.value().update();
	}

	if (pressed() && triggers.consume(SwitchButtonState::pressed)) { svc.soundboard.flags.world.set(audio::World::switch_press); }

	state_function = state_function();
}

void SwitchButton::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider); }

void SwitchButton::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
		sensorbox.setPosition(sensor.get_position() - cam);
		sensorbox.setSize(sensor.get_dimensions());
		sensorbox.setFillColor(sf::Color::Transparent);
		sensorbox.setOutlineThickness(-4);
		pressed() ? sensorbox.setOutlineColor(colors::dark_fucshia) : sensorbox.setOutlineColor(colors::mythic_green);
		win.draw(sensorbox);
		return;
	}
	sprite.render(svc, win, cam);
}

void SwitchButton::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (proj.get_bounding_box().overlaps(collider.bounding_box)) {
		state = SwitchButtonState::pressed;
		if (!proj.destruction_initiated()) { svc.soundboard.flags.world.set(audio::World::breakable_hit); }
		proj.destroy(false);
	}
}

fsm::StateFunction SwitchButton::update_unpressed() {
	external = SwitchButtonState::unpressed;
	sensor.set_position(collider.physics.position + sf::Vector2f{2.f, 0.f});
	if (sprite.just_started()) { shine_cooldown.start(); }
	shine_cooldown.update();
	if (change_state(SwitchButtonState::pressed, "squished")) {
		collider.dimensions.y = 10.f;
		collider.physics.position.y += 4.f;
		return SWITCH_BIND(update_squished);
	}
	if (shine_cooldown.is_complete()) {
		if (change_state(SwitchButtonState::unpressed, "shine")) { return SWITCH_BIND(update_shining); }
	}
	state = SwitchButtonState::unpressed;
	return SWITCH_BIND(update_unpressed);
}

fsm::StateFunction SwitchButton::update_shining() {
	external = SwitchButtonState::unpressed;
	sensor.set_position(collider.physics.position + sf::Vector2f{2.f, 0.f});
	if (change_state(SwitchButtonState::pressed, "squished")) {
		collider.dimensions.y = 10.f;
		collider.physics.position.y += 4.f;
		return SWITCH_BIND(update_squished);
	}
	if (sprite.complete()) {
		if (change_state(SwitchButtonState::unpressed, "neutral")) { return SWITCH_BIND(update_unpressed); }
	}
	return SWITCH_BIND(update_shining);
}

fsm::StateFunction SwitchButton::update_squished() {
	external = SwitchButtonState::unpressed;
	sensor.set_position(collider.physics.position + sf::Vector2f{2.f, -4.f});
	if (sprite.complete()) {
		if (change_state(SwitchButtonState::pressed, "pressed")) {
			collider.dimensions.y = 4.f;
			collider.physics.position.y += 6.f;
			return SWITCH_BIND(update_pressed);
		}
		if (change_state(SwitchButtonState::unpressed, "neutral")) {
			collider.dimensions.y = 14.f;
			collider.physics.position.y -= 4.f;
			return SWITCH_BIND(update_unpressed);
		}
	}
	return SWITCH_BIND(update_squished);
}

fsm::StateFunction SwitchButton::update_pressed() {
	external = SwitchButtonState::pressed;
	if (sprite.just_started()) { triggers.set(SwitchButtonState::pressed); }
	sensor.set_position(collider.physics.position + sf::Vector2f{2.f, -10.f});
	if (change_state(SwitchButtonState::unpressed, "rising")) {
		collider.dimensions.y = 10.f;
		collider.physics.position.y -= 6.f;
		return SWITCH_BIND(update_rising);
	}
	state = SwitchButtonState::pressed;
	return SWITCH_BIND(update_pressed);
}

fsm::StateFunction SwitchButton::update_rising() {
	sensor.set_position(collider.physics.position + sf::Vector2f{2.f, -4.f});
	external = SwitchButtonState::unpressed;
	if (sprite.complete()) {
		if (change_state(SwitchButtonState::unpressed, "neutral")) {
			collider.dimensions.y = 14.f;
			collider.physics.position.y -= 4.f;
			return SWITCH_BIND(update_unpressed);
		}
		if (change_state(SwitchButtonState::pressed, "pressed")) {
			collider.dimensions.y = 4.f;
			collider.physics.position.y += 6.f;
			return SWITCH_BIND(update_pressed);
		}
	}
	return SWITCH_BIND(update_rising);
}

bool SwitchButton::change_state(SwitchButtonState next, std::string_view tag) {
	if (state == next) {
		sprite.set_params(tag, true);
		return true;
	}
	return false;
}

} // namespace fornani::world
