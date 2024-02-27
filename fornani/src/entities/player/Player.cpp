
#include "Player.hpp"
#include "../../setup/LookupTables.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace player {

Player::Player() {}

void Player::init() {

	svc::dataLocator.get().load_player_params();

	collider = shape::Collider(sf::Vector2<float>{PLAYER_WIDTH, PLAYER_HEIGHT}, sf::Vector2<float>{PLAYER_START_X, PLAYER_START_Y});
	collider.physics = components::PhysicsComponent({physics_stats.ground_fric, physics_stats.ground_fric}, physics_stats.mass);

	collider.physics.set_constant_friction({physics_stats.ground_fric, physics_stats.air_fric});

	anchor_point = {collider.physics.position.x + PLAYER_WIDTH / 2, collider.physics.position.y + PLAYER_HEIGHT / 2};
	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::dark_orange, antenna_force));
	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::dark_orange, antenna_force, {2.f, 4.f}));

	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::bright_orange, antenna_force));
	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::bright_orange, antenna_force, {2.f, 4.f}));

	float back_fric{0.74f};
	float front_fric{0.77f};

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

void Player::update(Time dt) {

	collider.physics.gravity = physics_stats.grav;
	collider.physics.maximum_velocity = physics_stats.maximum_velocity;
	collider.physics.ground_friction = {physics_stats.ground_fric, physics_stats.ground_fric};
	collider.physics.air_friction = {physics_stats.air_fric, physics_stats.air_fric};

	update_direction();
	grounded() ? controller.ground() : controller.unground();

	controller.update();

	// update loadout
	if (!weapons_hotbar.empty()) {
		if (controller.arms_switch() == -1.f) {
			current_weapon--;
			if (current_weapon < 0) { current_weapon = (int)weapons_hotbar.size() - 1; }
			loadout.equipped_weapon = weapons_hotbar.at(current_weapon);
			flags.sounds.set(Soundboard::weapon_swap);
		}
		if (controller.arms_switch() == 1.f) {
			current_weapon++;
			if (current_weapon > weapons_hotbar.size() - 1) { current_weapon = 0; }
			loadout.equipped_weapon = weapons_hotbar.at(current_weapon);
			flags.sounds.set(Soundboard::weapon_swap);
		}
	}

	if (!flags.input.test(Input::restricted)) {

		if (moving() || flags.movement.test(Movement::look_up) || jump_request > -1) {
			flags.input.reset(Input::inspecting);
			flags.input.reset(Input::inspecting_trigger);
		}
		// jump!
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
		} else if (controller.jump_released() && controller.jumping() && !controller.jump_held() && collider.physics.velocity.y < 0) {
			collider.physics.acceleration.y *= physics_stats.jump_release_multiplier;
			controller.reset_jump();
		}

		// check keystate
		if (!controller.jumpsquatting()) { walk(); }

		// zero the player's horizontal acceleration if movement was not requested
		if (!moving()) {
			collider.physics.acceleration.x = 0.0f;
			if (abs(collider.physics.velocity.x) > stopped_threshold && grounded()) { flags.movement.set(Movement::just_stopped); }
		}

		// dash
		if (animation.state.test(AnimState::dash) || controller.dash_requested()) {
			collider.physics.acceleration.y = controller.vertical_movement() * physics_stats.dash_multiplier;
			collider.physics.velocity.y = controller.vertical_movement() * physics_stats.dash_multiplier;
			collider.physics.acceleration.x += controller.dash_value() * (physics_stats.maximum_velocity.x / physics_stats.air_multiplier) * physics_stats.dash_speed;
			collider.physics.velocity.x += controller.dash_value() * (physics_stats.maximum_velocity.x / physics_stats.air_multiplier) * 10.f;
			controller.dash();
		}

		// weapon physics
		if (controller.shot() && !weapons_hotbar.empty()) {
			if (controller.direction.und == dir::UND::down) { collider.physics.acceleration.y += -loadout.get_equipped_weapon().attributes.recoil / 80; }
			if (controller.direction.und == dir::UND::up) { collider.physics.acceleration.y += loadout.get_equipped_weapon().attributes.recoil; }
		}

		if (flags.movement.test(Movement::move_left) && flags.movement.test(Movement::move_right)) { collider.physics.acceleration.x = 0.0f; }
	}

	collider.physics.update_euler();
	collider.sync_components();

	// for parameter tweaking, remove later
	collider.update();

	update_invincibility();

	play_sounds();
	apparent_position.x = collider.physics.position.x + PLAYER_WIDTH / 2;
	apparent_position.y = collider.physics.position.y;
	
	update_animation();

	
	if (!animation.state.test(AnimState::dash) && !controller.dash_requested()) {
		controller.stop_dashing();
		controller.cancel_dash_request();
	}

	// antennae!
	update_antennae();

	if (!weapons_hotbar.empty()) {
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
			antennae[0].render(win, campos);
			antennae[2].render(win, campos);
			win.draw(sprite);
			antennae[1].render(win, campos);
			antennae[3].render(win, campos);
			svc::counterLocator.get().at(svc::draw_calls) += 5;
		}
	}

	if (!weapons_hotbar.empty()) {
		loadout.get_equipped_weapon().sp_gun.setTexture(lookup::weapon_texture.at(loadout.get_equipped_weapon().type));
		loadout.get_equipped_weapon().render(win, campos);
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
		controller.reset_dash_count();
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
	if (weapons_hotbar.empty()) { sprite.setTexture(svc::assetLocator.get().t_nani_unarmed); }
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
	loadout.get_equipped_weapon().update();
	if (controller.facing_right()) {
		hand_position = {28, 36};
	} else {
		hand_position = {20, 36};
	}
}

void Player::walk() {

	if (controller.moving_right() && !collider.flags.test(shape::State::has_right_collision)) { collider.physics.acceleration.x = grounded() ? physics_stats.x_acc : (physics_stats.x_acc / physics_stats.air_multiplier); }
	if (controller.moving_left() && !collider.flags.test(shape::State::has_left_collision)) { collider.physics.acceleration.x = grounded() ? -physics_stats.x_acc : (-physics_stats.x_acc / physics_stats.air_multiplier); }
	if (animation.get_frame() == 44 || animation.get_frame() == 46) {
		if (animation.animation.keyframe_over() && animation.state.test(AnimState::run)) { flags.sounds.set(Soundboard::step); }
	}
}

void Player::autonomous_walk() {
	collider.physics.acceleration.x = grounded() ? physics_stats.x_acc : (physics_stats.x_acc / physics_stats.air_multiplier);
	if (controller.facing_left()) { collider.physics.acceleration.x *= -1.f; }
	flags.movement.set(Movement::autonomous_walk);
	if (animation.get_frame() == 44 || animation.get_frame() == 46) {
		if (animation.animation.keyframe_over() && animation.state.test(AnimState::run)) { flags.sounds.set(Soundboard::step); }
	}
}

void Player::hurt(int amount = 1) {

	if (!is_invincible()) {
		player_stats.health -= amount;
		collider.physics.acceleration.y = -physics_stats.hurt_acc;
		collider.spike_trigger = false;
		make_invincible();
		flags.sounds.set(Soundboard::hurt);
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

void Player::restrict_inputs() {
	flags.input.set(Input::restricted);
	flags.movement.reset(Movement::look_down);
	flags.movement.reset(Movement::look_up);
	flags.input.reset(Input::inspecting_trigger);
	controller.set_shot(false);
}

void Player::unrestrict_inputs() {
	flags.input.reset(Input::restricted);
	flags.input.reset(Input::no_anim);
	flags.movement.reset(Movement::autonomous_walk);
}

void Player::restrict_animation() { flags.input.set(Input::no_anim); }

void Player::no_move() {
	flags.movement.reset(Movement::move_right);
	flags.movement.reset(Movement::move_left);
	controller.stop();
}

bool Player::grounded() const { return collider.flags.test(shape::State::grounded); }

bool Player::moving() { return (controller.moving() || flags.movement.test(Movement::autonomous_walk)); }

bool Player::moving_at_all() { return (controller.moving() || flags.movement.test(Movement::autonomous_walk) || flags.movement.test(Movement::freefalling) || flags.movement.test(Movement::entered_freefall)); }

bool Player::can_shoot() { return controller.shot() && !loadout.get_equipped_weapon().cooling_down(); }

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
	weapons_hotbar.clear();
	update_antennae();
}

dir::LR Player::entered_from() { return (collider.physics.position.x < lookup::SPACING * 8) ? dir::LR::right : dir::LR::left; }

void Player::play_sounds() {

	if (flags.sounds.test(Soundboard::land)) { svc::assetLocator.get().landed.play(); }
	if (flags.sounds.test(Soundboard::jump)) { svc::assetLocator.get().jump.play(); }
	if (flags.sounds.test(Soundboard::step)) {
		util::Random r{};
		float randp = r.random_range_float(0.0f, 0.1f);
		svc::assetLocator.get().step.setPitch(1.0f + randp);

		svc::assetLocator.get().step.setVolume(60);
		svc::assetLocator.get().step.play();
	}
	if (flags.sounds.test(Soundboard::weapon_swap)) { svc::assetLocator.get().arms_switch.play(); }
	if (flags.sounds.test(Soundboard::hurt)) { svc::assetLocator.get().hurt.play(); }
	flags.sounds = {};
}

std::string Player::print_direction(bool lr) {
	if (lr) {
		if (controller.facing_left()) return "LEFT";
		if (controller.facing_right()) return "RIGHT";
	}
}

} // namespace player