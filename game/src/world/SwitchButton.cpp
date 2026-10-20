
#include "fornani/world/SwitchButton.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Breakable.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

SwitchButton::SwitchButton(automa::ServiceProvider& svc, sf::Vector2f position, int id, int type, Map& map) : Animatable(svc, "switch_buttons", {18, 12}), id(id), type(static_cast<SwitchType>(type)) {
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
	push_and_set_animation("neutral", {0, 1, 28, -1});
	push_animation("shine", {1, 4, 28, 0});
	push_animation("squished", {5, 1, 28, 0, false, true});
	push_animation("pressed", {6, 1, 28, 0, true});
	push_animation("rising", {7, 1, 28, 0, false, true});
	set_origin({9.f, 11.f});
	auto const style = map.get_biome_properties()["switches"]["channel"].as<int>();
	set_channel(type + style * static_cast<int>(SwitchType::END));
	if (svc.data.switch_is_activated(id)) {
		state = SwitchButtonState::pressed;
		m_flags.set(SwitchButtonState::pressed);
		state_function = std::bind(&SwitchButton::update_pressed, this);
		collider.dimensions.y = 4.f;
		collider.physics.position.y += 6.f;
		set_animation("pressed");
		for (auto& block : map.switch_blocks) {
			if (block->get_id() == id && pressed()) { block->turn_off(); }
		}
	}
}

void SwitchButton::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (collider.collision_depths) { collider.collision_depths.value().reset(); }
	tick();

	for (auto& block : map.switch_blocks) {
		if (type == SwitchType::permanent) { continue; }
		if (matches(*block)) { pressed() ? block->turn_off() : block->turn_on(); }
		if (block->get_type() == SwitchType::alternator && type == SwitchType::alternator) {
			if (block->get_id() == id + 1) { pressed() ? block->turn_on() : block->turn_off(); }
		}
	}

	// type-specific stuff
	if (type == SwitchType::movable) {
		if (player.get_collider().predictive_horizontal.overlaps(collider.bounding_box)) { collider.physics.velocity.x = player.get_collider().physics.acceleration.x * 0.5f; }
		collider.update(svc);
		collider.handle_collider_collision(player.get_collider());
	}

	if (m_flags.test(SwitchButtonState::triggered)) {
		state = SwitchButtonState::pressed;
		auto const& biome = map.get_biome_properties()["switches"]["sound"];
		auto const sound = biome ? biome.as_string() : "switch_press";
		svc.soundboard.play_sound(sound);
		m_flags.reset(SwitchButtonState::triggered);
		if (type == SwitchType::permanent) { svc.events.press_permanent_switch_event.dispatch(svc, id); }
	}

	// press permanent switches forever and use cutscene events to do it
	if (type == SwitchType::permanent) {
		if (svc.data.switch_is_activated(id) && !m_flags.test(SwitchButtonState::pressed)) {
			m_flags.set(SwitchButtonState::pressed);
			for (auto& block : map.switch_blocks) {
				if (matches(*block)) { block->turn_off(); }
			}
		}
	}

	// assume unpressed, then check everything for a press
	for (auto& breakable : map.breakables) { collider.handle_shape_collision(breakable->get_bounding_box()); }
	auto was_pressed = false;
	for (auto& platform : map.platforms) {
		if (platform->get_collider().bounding_box.overlaps(sensor)) { was_pressed = true; }
	}
	for (auto& chest : map.chests) {
		if (chest->get_collider().collides_with(sensor)) { was_pressed = true; }
	}
	for (auto& pushable : map.pushables) {
		if (pushable->get_collider().jumpbox.overlaps(sensor)) { was_pressed = true; }
	}
	if (player.get_collider().jumpbox.overlaps(sensor)) { was_pressed = true; }
	if (was_pressed && !pressed()) { m_flags.set(SwitchButtonState::triggered); }
	if (!was_pressed && pressed() && type != SwitchType::permanent) { state = SwitchButtonState::unpressed; }

	collider.detect_map_collision(map);
	handle_collision(player.get_collider());
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};
	if (collider.collision_depths) { collider.collision_depths.value().update(); }

	state_function = state_function();
}

void SwitchButton::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider); }

void SwitchButton::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (!debug::is_production()) {
		collider.render(win, cam);
		sensorbox.setPosition(sensor.get_position() - cam);
		sensorbox.setSize(sensor.get_dimensions());
		sensorbox.setFillColor(sf::Color::Transparent);
		sensorbox.setOutlineThickness(-4);
		pressed() ? sensorbox.setOutlineColor(colors::dark_fucshia) : sensorbox.setOutlineColor(colors::mythic_green);
		win.draw(sensorbox);
		return;
	}
	set_position(collider.get_bottom() - cam);
	win.draw(*this);
}

void SwitchButton::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (proj.get_collider().collides_with(collider.bounding_box)) {
		state = SwitchButtonState::pressed;
		if (!proj.destruction_initiated()) { svc.soundboard.flags.world.set(audio::World::breakable_hit); }
		proj.destroy(false);
	}
}

fsm::StateFunction SwitchButton::update_unpressed() {
	external = SwitchButtonState::unpressed;
	sensor.set_position(collider.physics.position + sf::Vector2f{2.f, 0.f});
	if (animation.just_started()) { shine_cooldown.start(); }
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
	if (animation.is_complete()) {
		if (change_state(SwitchButtonState::unpressed, "neutral")) { return SWITCH_BIND(update_unpressed); }
	}
	return SWITCH_BIND(update_shining);
}

fsm::StateFunction SwitchButton::update_squished() {
	external = SwitchButtonState::unpressed;
	sensor.set_position(collider.physics.position + sf::Vector2f{2.f, -4.f});
	if (animation.complete()) {
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
	if (animation.complete()) {
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
		animation.set_params(get_params(std::string(tag)));
		return true;
	}
	return false;
}

bool SwitchButton::matches(SwitchBlock& block) { return block.get_id() == id && block.get_type() == type; }

} // namespace fornani::world
