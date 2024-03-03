
#include "Player.hpp"
#include "../../setup/LookupTables.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace player {

Player::Player() {}

void Player::init() {

	svc::dataLocator.get().load_player_params();
	arsenal.init();

	collider = shape::Collider(sf::Vector2<float>{PLAYER_WIDTH, PLAYER_HEIGHT}, sf::Vector2<float>{PLAYER_START_X, PLAYER_START_Y});
	collider.physics = components::PhysicsComponent({physics_stats.ground_fric, physics_stats.ground_fric}, physics_stats.mass);

	collider.physics.set_constant_friction({physics_stats.ground_fric, physics_stats.air_fric});

	anchor_point = {collider.physics.position.x + PLAYER_WIDTH / 2, collider.physics.position.y + PLAYER_HEIGHT / 2};
	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::dark_orange, antenna_force));
	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::dark_orange, antenna_force, {2.f, 4.f}));

	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::bright_orange, antenna_force));
	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::bright_orange, antenna_force, {2.f, 4.f}));

	float back_fric{0.84f};
	float front_fric{0.87f};

	antennae[0].collider.physics = components::PhysicsComponent(sf::Vector2<float>{back_fric, back_fric}, 1.0f);
	antennae[0].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);
	antennae[1].collider.physics = components::PhysicsComponent(sf::Vector2<float>{back_fric, back_fric}, 1.0f);
	antennae[1].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);

	antennae[2].collider.physics = components::PhysicsComponent(sf::Vector2<float>{front_fric, front_fric}, 1.0f);
	antennae[2].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);
	antennae[3].collider.physics = components::PhysicsComponent(sf::Vector2<float>{front_fric, front_fric}, 1.0f);
	antennae[3].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);

	sprite_dimensions = {48.f, 48.f};

	// sprites
	assign_texture(svc::assetLocator.get().t_nani_unarmed);
	sprite.setTexture(svc::assetLocator.get().t_nani_unarmed);
}

void Player::update() {

	collider.physics.gravity = physics_stats.grav;
	collider.physics.maximum_velocity = physics_stats.maximum_velocity;
	collider.physics.ground_friction = {physics_stats.ground_fric, physics_stats.ground_fric};
	collider.physics.air_friction = {physics_stats.air_fric, physics_stats.air_fric};

	update_direction();
	grounded() ? controller.ground() : controller.unground();
	controller.update();
	update_transponder();

	if (grounded()) { controller.reset_dash_count(); }

	// update loadout
	if (!arsenal.loadout.empty()) {
		if (controller.arms_switch() == -1.f) {
			current_weapon--;
			if (current_weapon < 0) { current_weapon = (int)arsenal.loadout.size() - 1; }
			// loadout.equipped_weapon = weapons_hotbar.at(current_weapon);
			svc::soundboardLocator.get().player.set(audio::Player::arms_switch);
		}
		if (controller.arms_switch() == 1.f) {
			current_weapon++;
			if (current_weapon > arsenal.loadout.size() - 1) { current_weapon = 0; }
			// loadout.equipped_weapon = weapons_hotbar.at(current_weapon);
			svc::soundboardLocator.get().player.set(audio::Player::arms_switch);
		}
	}

	//do this elsehwere later
	if (collider.flags.test(shape::State::just_landed)) { svc::soundboardLocator.get().player.set(audio::Player::land); }
	collider.flags.reset(shape::State::just_landed);

	jump();
	dash();

	// check keystate
	if (!controller.jumpsquatting()) { walk(); }
	if (!controller.moving()) { collider.physics.acceleration.x = 0.0f; }


	// weapon physics
	if (controller.shot() && !arsenal.loadout.empty()) {
		if (controller.direction.und == dir::UND::down) { collider.physics.acceleration.y += -equipped_weapon().attributes.recoil / 80; }
		if (controller.direction.und == dir::UND::up) { collider.physics.acceleration.y += equipped_weapon().attributes.recoil; }
	}

	collider.physics.update_euler();
	collider.sync_components();

	// for parameter tweaking, remove later
	collider.update();

	update_invincibility();

	apparent_position.x = collider.physics.position.x + PLAYER_WIDTH / 2;
	apparent_position.y = collider.physics.position.y;

	update_animation();
	update_weapon();

	if (!animation.state.test(AnimState::dash) && !controller.dash_requested()) {
		controller.stop_dashing();
		controller.cancel_dash_request();
	}

	// antennae!
	update_antennae();

	if (!arsenal.loadout.empty()) {
		assign_texture(svc::assetLocator.get().t_nani);
		sprite.setTexture(svc::assetLocator.get().t_nani);
	} else {
		assign_texture(svc::assetLocator.get().t_nani_unarmed);
		sprite.setTexture(svc::assetLocator.get().t_nani_unarmed);
	}
}

void Player::render(sf::RenderWindow& win, sf::Vector2<float>& campos) {

	sf::Vector2<float> player_pos = apparent_position - campos;
	calculate_sprite_offset();

	// get UV coords
	int u = (int)(animation.get_frame() / asset::NANI_SPRITESHEET_HEIGHT) * asset::NANI_SPRITE_WIDTH;
	int v = (int)(animation.get_frame() % asset::NANI_SPRITESHEET_HEIGHT) * asset::NANI_SPRITE_WIDTH;
	sprite.setTextureRect(sf::IntRect({u, v}, {asset::NANI_SPRITE_WIDTH, asset::NANI_SPRITE_WIDTH}));
	sprite.setOrigin(asset::NANI_SPRITE_WIDTH / 2, asset::NANI_SPRITE_WIDTH / 2);
	sprite.setPosition(sprite_position.x - campos.x, sprite_position.y - campos.y);

	// flip the sprite based on the player's direction
	sf::Vector2<float> right_scale = {1.0f, 1.0f};
	sf::Vector2<float> left_scale = {-1.0f, 1.0f};
	if (controller.facing_left() && sprite.getScale() == right_scale) {
		sprite.scale(-1.0f, 1.0f);
		if (grounded()) { animation.state.set(AnimState::turn); }
	}
	if (controller.facing_right() && sprite.getScale() == left_scale) {
		sprite.scale(-1.0f, 1.0f);
		if (grounded()) { animation.state.set(AnimState::turn); }
	}
	if (flags.state.test(State::alive)) {
		if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
			collider.render(win, campos);
		} else {
			antennae[1].render(win, campos);
			antennae[3].render(win, campos);
			win.draw(sprite);
			antennae[0].render(win, campos);
			antennae[2].render(win, campos);
			svc::counterLocator.get().at(svc::draw_calls) += 5;
		}
	}

	if (!arsenal.loadout.empty()) {
		equipped_weapon().sp_gun.setTexture(lookup::weapon_texture.at(equipped_weapon().type));
		equipped_weapon().render(win, campos);
	}
}

void Player::assign_texture(sf::Texture& tex) { sprite.setTexture(tex); }

void Player::update_animation() {

	if (grounded()) {
		if (controller.inspecting()) { animation.state.set(AnimState::inspect); }
		if (!(animation.state.test(AnimState::jumpsquat) || animation.state.test(AnimState::land) || animation.state.test(AnimState::rise))) {
			if (controller.inspecting()) { animation.state.set(AnimState::inspect); }
			if (controller.nothing_pressed() && !controller.dashing()) { animation.state.set(AnimState::idle); }
			if (controller.moving() && !controller.dashing()) { animation.state.set(AnimState::run); }
			if (abs(collider.physics.velocity.x) > animation.stop_threshold) { animation.state.test(AnimState::stop); }
		}
	} else {
		if (collider.physics.velocity.y > -animation.suspension_threshold && collider.physics.velocity.y < animation.suspension_threshold) { animation.state.set(AnimState::suspend); }
	}

	if (collider.flags.test(shape::State::just_landed)) {
		animation.state.set(AnimState::land);
	}
	if (animation.state.test(AnimState::fall) && grounded()) { animation.state.set(AnimState::land); }
	if (animation.state.test(AnimState::suspend) && grounded()) { animation.state.set(AnimState::land); }

	if (collider.physics.velocity.y < -animation.suspension_threshold && !grounded()) { animation.state.set(AnimState::rise); }
	if (collider.physics.velocity.y > animation.suspension_threshold && !grounded()) { animation.state.set(AnimState::fall); }

	if (controller.dashing() && controller.can_dash()) { animation.state.set(AnimState::dash); }
	if (controller.dash_requested()) { animation.state.set(AnimState::dash); }

	animation.update();
}

void Player::update_sprite() {
	if (arsenal.loadout.empty()) { sprite.setTexture(svc::assetLocator.get().t_nani_unarmed); }
}

void Player::update_transponder() {
	if (svc::consoleLocator.get().flags.test(gui::ConsoleFlags::active)) {
		controller.prevent_movement();
		if (controller.transponder_skip()) { transponder.skip_ahead(); }
		if (controller.transponder_next()) { transponder.next(); }
		if (controller.transponder_exit()) { transponder.exit(); }
		transponder.update();
	}
	transponder.end();
}

void Player::flash_sprite() {
	if ((counters.invincibility / 10) % 2 == 0) {
		sprite.setColor(flcolor::red);
	} else {
		sprite.setColor(flcolor::blue);
	}
}

void Player::calculate_sprite_offset() {
	sprite_offset.y = 0.f;
	if (collider.flags.test(shape::State::on_ramp)) { sprite_offset.y = -2.f; }
	sprite_position = {collider.physics.position.x + 9.f, collider.physics.position.y + sprite_offset.y};
}

void Player::jump() {
	if (controller.jumpsquat_trigger()) {
		animation.state.set(AnimState::jumpsquat);
		controller.start_jumpsquat();
		controller.reset_jumpsquat_trigger();
	}
	if (controller.jumpsquatting() && !animation.state.test(AnimState::jumpsquat)) {
		controller.stop_jumpsquatting();
		controller.start_jumping();
		collider.physics.acceleration.y = -physics_stats.jump_velocity;
		animation.state.set(AnimState::rise);
		svc::soundboardLocator.get().player.set(audio::Player::jump);
	} else if (controller.jump_released() && controller.jumping() && !controller.jump_held() && collider.physics.velocity.y < 0) {
		collider.physics.acceleration.y *= physics_stats.jump_release_multiplier;
		controller.reset_jump();
	}
}

void Player::dash() {
	if (animation.state.test(AnimState::dash) || controller.dash_requested()) {
		collider.physics.acceleration.y = controller.vertical_movement() * physics_stats.vertical_dash_multiplier;
		collider.physics.velocity.y = controller.vertical_movement() * physics_stats.vertical_dash_multiplier;

		if (!collider.has_horizontal_collision()) {
			collider.physics.acceleration.x += controller.dash_value() * physics_stats.dash_speed;
			collider.physics.velocity.x += controller.dash_value() * physics_stats.dash_speed;
		}
		controller.dash();
	}
}

void Player::set_position(sf::Vector2<float> new_pos) {
	collider.physics.position = new_pos;
	collider.sync_components();
	int ctr{0};
	for (auto& a : antennae) {
		a.update();
		a.collider.physics.position = collider.physics.position + antenna_offset;
		antenna_offset.x = ctr % 2 == 0 ? 18.0f : 7.0f;
		++ctr;
	}
}

void Player::update_direction() {
	if (controller.facing_left()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2 - ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	} else if (controller.facing_right()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2 + ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	} else {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	}
}

void Player::update_weapon() {
	equipped_weapon().update();
	if (controller.facing_right()) {
		hand_position = {28, 36};
	} else {
		hand_position = {20, 36};
	}
}

void Player::walk() {
	if (controller.moving_right() && !collider.has_right_collision()) { collider.physics.acceleration.x = grounded() ? physics_stats.x_acc : (physics_stats.x_acc / physics_stats.air_multiplier); }
	if (controller.moving_left() && !collider.has_left_collision()) { collider.physics.acceleration.x = grounded() ? -physics_stats.x_acc : (-physics_stats.x_acc / physics_stats.air_multiplier); }
	if (animation.get_frame() == 44 || animation.get_frame() == 46) {
		if (animation.animation.keyframe_over() && animation.state.test(AnimState::run)) { svc::soundboardLocator.get().player.set(audio::Player::step); }
	}
}

void Player::hurt(int amount = 1) {

	if (!is_invincible()) {
		player_stats.health -= amount;
		collider.physics.acceleration.y = -physics_stats.hurt_acc;
		collider.spike_trigger = false;
		make_invincible();
		svc::soundboardLocator.get().player.set(audio::Player::hurt);
		just_hurt = true;
	}

	if (player_stats.health <= 0) { kill(); }
}

void Player::update_antennae() {
	int ctr{0};
	for (auto& a : antennae) {
		a.set_target_position(collider.physics.position + antenna_offset);
		a.update();
		a.collider.sync_components();
		if (controller.facing_right()) {
			antenna_offset.x = ctr % 2 == 0 ? 18.0f : 7.f;
		} else {
			antenna_offset.x = ctr % 2 == 0 ? 2.f : 13.f;
		}
		++ctr;
	}
}

bool Player::grounded() const { return collider.flags.test(shape::State::grounded); }

bool Player::fire_weapon() {
	if (controller.shot() && !equipped_weapon().cooling_down()) {
		svc::soundboardLocator.get().weapon.set(lookup::gun_sound.at(equipped_weapon().type));
		return true;
	}
	return false;
}

void Player::make_invincible() { counters.invincibility = INVINCIBILITY_TIME; }

void Player::update_invincibility() {

	if (is_invincible()) {
		flash_sprite();
	} else {
		sprite.setColor(sf::Color::White);
	}
	--counters.invincibility;
	if (counters.invincibility < 0) { counters.invincibility = 0; }
}

bool Player::is_invincible() const { return counters.invincibility > 0; }

void Player::kill() { flags.state.reset(State::alive); }

void Player::start_over() {
	player_stats.health = player_stats.max_health;
	flags.state.set(State::alive);
}

void Player::reset_flags() { flags = {}; }

void Player::total_reset() {
	start_over();
	collider.physics.zero();
	reset_flags();
	arsenal.loadout.clear();
	update_antennae();
}

arms::Weapon& Player::equipped_weapon() {
	if (arsenal.loadout.empty()) {
		// default to bryns gun
		return arsenal.armory.at(0);
	} else {
		return arsenal.loadout.at(current_weapon);
	}
}

dir::LR Player::entered_from() { return (collider.physics.position.x < lookup::SPACING * 8) ? dir::LR::right : dir::LR::left; }

std::string Player::print_direction(bool lr) {
	if (lr) {
		if (controller.facing_left()) return "LEFT";
		if (controller.facing_right()) return "RIGHT";
	}
}

} // namespace player