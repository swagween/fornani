
#include "Player.hpp"
#include "../../setup/LookupTables.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace player {

Player::Player() {

	collider = shape::Collider(sf::Vector2<float>{PLAYER_WIDTH, PLAYER_HEIGHT}, sf::Vector2<float>{PLAYER_START_X, PLAYER_START_Y});
	collider.physics = components::PhysicsComponent({stats.PLAYER_GROUND_FRIC, stats.PLAYER_GROUND_FRIC}, stats.PLAYER_MASS);
	head = shape::Collider(sf::Vector2<float>{PLAYER_WIDTH + 4, head_height}, sf::Vector2<float>{PLAYER_START_X - 2, PLAYER_START_Y - head_height});
	collider.physics = components::PhysicsComponent({stats.PLAYER_GROUND_FRIC, stats.PLAYER_GROUND_FRIC}, stats.PLAYER_MASS);

	anchor_point = {collider.physics.position.x + PLAYER_WIDTH / 2, collider.physics.position.y + PLAYER_HEIGHT / 2};
	behavior.current_state = behavior::Behavior(behavior::idle);
	behavior.facing_lr = behavior::DIR_LR::RIGHT;
	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::dark_orange, 0.029f));
	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::dark_orange, 0.029f, {2.f, 4.f}));

	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::bright_orange, 0.03f));
	antennae.push_back(vfx::Attractor(collider.physics.position, flcolor::bright_orange, 0.03f, {2.f, 4.f}));

	antennae[0].collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.73f, 0.73f}, 1.0f);
	antennae[0].collider.physics.maximum_velocity = sf::Vector2<float>(2.5f, 2.5f);
	antennae[1].collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.73f, 0.73f}, 1.0f);
	antennae[1].collider.physics.maximum_velocity = sf::Vector2<float>(2.5f, 2.5f);

	antennae[2].collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.75f, 0.75f}, 1.0f);
	antennae[2].collider.physics.maximum_velocity = sf::Vector2<float>(4.5f, 4.5f);
	antennae[3].collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.75f, 0.75f}, 1.0f);
	antennae[3].collider.physics.maximum_velocity = sf::Vector2<float>(4.5f, 4.5f);

	sprite_dimensions = {48.f, 48.f};

	// sprites
	assign_texture(svc::assetLocator.get().t_nani_unarmed);
	sprite.setTexture(svc::assetLocator.get().t_nani_unarmed);
}

void Player::handle_events(sf::Event& event) {
	if (!svc::consoleLocator.get().flags.test(gui::ConsoleFlags::active)) { unrestrict_inputs(); }
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::triggered)) {
		flags.movement.set(Movement::move_left);
		if (grounded()) {
			if (behavior.facing == behavior::DIR::RIGHT) {
				behavior.turn();
				behavior.facing = behavior::DIR::LEFT;
				flags.movement.reset(Movement::is_wall_sliding);
			} else {
				behavior.run();
			}
		} else if (!flags.input.test(Input::restricted)) {
			if (!flags.movement.test(Movement::is_wall_sliding)) { behavior.air(collider.physics.velocity.y); }
		}
		behavior.facing = behavior::DIR::LEFT;
		behavior.facing_lr = behavior::DIR_LR::LEFT;
	}
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::triggered)) {
		flags.movement.set(Movement::move_right);
		if (grounded()) {
			if (behavior.facing == behavior::DIR::LEFT) {
				behavior.turn();
				flags.movement.reset(Movement::is_wall_sliding);
			} else {
				behavior.run();
			}
		} else if (!flags.input.test(Input::restricted)) {
			if (!flags.movement.test(Movement::is_wall_sliding)) { behavior.air(collider.physics.velocity.y); }
		}
		behavior.facing = behavior::DIR::RIGHT;
		behavior.facing_lr = behavior::DIR_LR::RIGHT;
	}
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Up).key_state.test(util::key_state::triggered)) { flags.movement.set(Movement::look_up); }
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::triggered)) { flags.movement.set(Movement::look_down); }
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::released)) {
		if (!flags.movement.test(Movement::autonomous_walk)) {
			flags.movement.reset(Movement::move_left);
			collider.flags.reset(shape::State::has_left_collision);
			if (!collider.flags.test(shape::State::has_right_collision)) { flags.movement.reset(Movement::is_wall_sliding); }
			flags.movement.set(Movement::stopping);
			last_dir = behavior::DIR::LEFT;
			flags.movement.set(Movement::left_released);
			if (grounded()) {
				if (!flags.movement.test(Movement::move_right) && !behavior.restricted()) { behavior.reset(); }
			} else {
				if (!behavior.restricted() && !flags.movement.test(Movement::is_wall_sliding)) { behavior.air(collider.physics.velocity.y); }
			}
			if (flags.movement.test(Movement::move_right)) { behavior.facing_lr = behavior::DIR_LR::RIGHT; }
		}
	}
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::released)) {
		flags.movement.reset(Movement::move_right);
		collider.flags.reset(shape::State::has_right_collision);
		if (!collider.flags.test(shape::State::has_left_collision)) { flags.movement.reset(Movement::is_wall_sliding); }
		flags.movement.set(Movement::stopping);
		last_dir = behavior::DIR::RIGHT;
		flags.movement.set(Movement::right_released);
		if (grounded()) {
			if (!behavior.restricted()) { behavior.reset(); }
		} else {
			if (!behavior.restricted() && !flags.movement.test(Movement::is_wall_sliding)) { behavior.air(collider.physics.velocity.y); }
		}
		if (flags.movement.test(Movement::move_left)) { behavior.facing_lr = behavior::DIR_LR::LEFT; }
	}
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Up).key_state.test(util::key_state::released)) { flags.movement.reset(Movement::look_up); }
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::released)) { flags.movement.reset(Movement::look_down); }
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::released)) { weapon_fired = false; }
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::triggered)) {
		if (!flags.input.test(Input::restricted)) {
			flags.jump.set(Jump::is_pressed);
			jump_request = JUMP_BUFFER_TIME;
			flags.jump.set(Jump::just_jumped);
			flags.jump.set(Jump::trigger);
		}
	}
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::released)) {
		if (flags.jump.test(Jump::just_jumped) || flags.jump.test(Jump::hold) || flags.jump.test(Jump::jumping) || jump_request > -1) { flags.jump.set(Jump::is_released); }
		flags.jump.reset(Jump::is_pressed);
		flags.jump.reset(Jump::hold);
		if (!grounded()) { jump_request = -1; }
		if (!behavior.restricted()) { flags.jump.set(Jump::can_jump); }
	}
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::triggered)) { weapon_fired = true; }
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::A).key_state.test(util::key_state::triggered)) {
		if (!weapons_hotbar.empty()) {
			current_weapon--;
			if (current_weapon < 0) { current_weapon = (int)weapons_hotbar.size() - 1; }
			loadout.equipped_weapon = weapons_hotbar.at(current_weapon);
			flags.sounds.set(Soundboard::weapon_swap);
		}
	}
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::S).key_state.test(util::key_state::triggered)) {
		if (!weapons_hotbar.empty()) {
			current_weapon++;
			if (current_weapon > weapons_hotbar.size() - 1) { current_weapon = 0; }
			loadout.equipped_weapon = weapons_hotbar.at(current_weapon);
			flags.sounds.set(Soundboard::weapon_swap);
		}
	}
	if (svc::inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::triggered)) {
		if (grounded()) { flags.input.set(Input::inspecting_trigger); }
	}
}

void Player::update(Time dt) {

	if (!flags.input.test(Input::no_anim)) {
		update_animation();
		update_sprite();
	}

	if (!flags.input.test(Input::restricted)) {

		if (moving() || flags.movement.test(Movement::look_up) || jump_request > -1) {
			flags.input.reset(Input::inspecting);
			flags.input.reset(Input::inspecting_trigger);
		}

		if (flags.input.test(Input::inspecting_trigger) && behavior.current_state.params.behavior_id == "inspecting" && behavior.current_state.params.done) { flags.input.set(Input::inspecting); }

		// check if player requested jump
		if (grounded() && jump_request > -1) {
			flags.jump.set(Jump::hold);
			if (flags.jump.test(Jump::just_jumped)) { behavior.jump(); }
		}

		// preset flags
		if (grounded()) { flags.jump.reset(Jump::jumping); } // do this before jumping, otherwise it will trigger on the same frame

		// jump!
		if (jump_request > -1) {
			if (!behavior.restricted()) { // once jumpsquat is over
				jump_request--;
				if (grounded()) {
					collider.physics.acceleration.y = -stats.JUMP_MAX / 1.3f;
					flags.jump.set(Jump::jumping);
					flags.jump.reset(Jump::can_jump);
					flags.jump.reset(Jump::trigger);
					behavior.rise();
					if (!flags.jump.test(Jump::trigger)) { flags.sounds.set(Soundboard::jump); }
				}
			}
		}

		// reset jump flags
		if (!flags.jump.test(Jump::is_pressed)) { flags.jump.reset(Jump::hold); }
		if (grounded() && jump_request == -1) { flags.jump.reset(Jump::is_released); }
		if (flags.jump.test(Jump::is_released) && flags.jump.test(Jump::jumping)) {
			collider.physics.acceleration.y *= stats.JUMP_RELEASE_MULTIPLIER;
			flags.jump.reset(Jump::jumping);
		} // player lets go
		if (flags.jump.test(Jump::is_released) && !grounded()) { flags.jump.reset(Jump::is_released); }

		// check keystate
		if (!behavior.restricted()) { walk(); }

		// zero the player's horizontal acceleration if movement was not requested
		if (!moving()) {
			collider.physics.acceleration.x = 0.0f;
			if (abs(collider.physics.velocity.x) > stopped_threshold && grounded()) { flags.movement.set(Movement::just_stopped); }
		}

		// weapon physics
		if (weapon_fired && !weapons_hotbar.empty()) {
			if (behavior.facing_strictly_right()) {
				if (!collider.flags.test(shape::State::has_right_collision)) { collider.physics.acceleration.x += -loadout.get_equipped_weapon().attributes.recoil; }
			}
			if (behavior.facing_strictly_left()) {
				if (!collider.flags.test(shape::State::has_left_collision)) { collider.physics.acceleration.x += loadout.get_equipped_weapon().attributes.recoil; }
			}
			if (behavior.facing_down()) { collider.physics.acceleration.y += -loadout.get_equipped_weapon().attributes.recoil / 8; }
			if (behavior.facing_up()) { collider.physics.acceleration.y += loadout.get_equipped_weapon().attributes.recoil; }
		}

		if (flags.movement.test(Movement::move_left) && flags.movement.test(Movement::move_right)) { collider.physics.acceleration.x = 0.0f; }
	}

	collider.physics.update_euler();
	collider.sync_components();
	head.physics.position = {collider.bounding_box.position.x - 2, collider.bounding_box.position.y - head_height};
	head.sync_components();

	// for parameter tweaking, remove later
	collider.physics.friction = grounded() ? sf::Vector2<float>{stats.PLAYER_GROUND_FRIC, stats.PLAYER_GROUND_FRIC} : sf::Vector2<float>{stats.PLAYER_HORIZ_AIR_FRIC, stats.PLAYER_VERT_AIR_FRIC};
	collider.update();

	update_invincibility();

	play_sounds();
	update_behavior();
	apparent_position.x = collider.physics.position.x + PLAYER_WIDTH / 2;
	apparent_position.y = collider.physics.position.y;

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
	int u = (int)(behavior.get_frame() / asset::NANI_SPRITESHEET_HEIGHT) * asset::NANI_SPRITE_WIDTH;
	int v = (int)(behavior.get_frame() % asset::NANI_SPRITESHEET_HEIGHT) * asset::NANI_SPRITE_WIDTH;
	sprite.setTextureRect(sf::IntRect({u, v}, {asset::NANI_SPRITE_WIDTH, asset::NANI_SPRITE_WIDTH}));
	sprite.setOrigin(asset::NANI_SPRITE_WIDTH / 2, asset::NANI_SPRITE_WIDTH / 2);
	sprite.setPosition(sprite_position.x - campos.x, sprite_position.y - campos.y);

	// flip the sprite based on the player's direction
	sf::Vector2<float> right_scale = {1.0f, 1.0f};
	sf::Vector2<float> left_scale = {-1.0f, 1.0f};
	if (behavior.facing_lr == behavior::DIR_LR::LEFT && sprite.getScale() == right_scale) {
		// loadout.get_equipped_weapon().sp_gun.scale(-1.0f, 1.0f);
		sprite.scale(-1.0f, 1.0f);
	}
	if (behavior.facing_lr == behavior::DIR_LR::RIGHT && sprite.getScale() == left_scale) {
		// loadout.get_equipped_weapon().sp_gun.scale(-1.0f, 1.0f);
		sprite.scale(-1.0f, 1.0f);
	}
	if (flags.state.test(State::alive)) {
		if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
			collider.render(win, campos);
			head.render(win, campos);
		} else {
			win.draw(sprite);
			svc::counterLocator.get().at(svc::draw_calls)++;
			// collider.render(win, campos);
			// head.render(win, campos);
		}
	}

	if (!weapons_hotbar.empty()) {
		loadout.get_equipped_weapon().sp_gun.setTexture(lookup::weapon_texture.at(loadout.get_equipped_weapon().type));
		loadout.get_equipped_weapon().render(win, campos);
	}

	for (auto& a : antennae) { a.render(win, campos); }
}

void Player::assign_texture(sf::Texture& tex) { sprite.setTexture(tex); }

void Player::update_animation() {
	behavior.end_loop();
	behavior.current_state.update();
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

void Player::update_behavior() {

	if (flags.jump.test(Jump::just_jumped) && !flags.movement.test(Movement::is_wall_sliding)) {
		behavior.air(collider.physics.velocity.y);
		flags.jump.reset(Jump::just_jumped);
	}

	if (behavior.current_state.params.complete) {
		if (grounded()) {
			behavior.reset();
		} else {
			behavior.air(collider.physics.velocity.y);
		}
	}

	if (moving() && behavior.current_state.params.behavior_id == "idle") {
		if (grounded()) {
			behavior.run();
		} else {
			behavior.air(collider.physics.velocity.y);
		}
	}

	if (flags.movement.test(Movement::just_stopped)) { behavior.stop(); }

	if (flags.input.test(Input::inspecting_trigger) && !(behavior.current_state.params.behavior_id == "inspecting")) { behavior.inspect(); }

	if (collider.physics.velocity.y > behavior.suspension_threshold && !flags.movement.test(Movement::freefalling)) { flags.movement.set(Movement::entered_freefall); }
	if (flags.movement.test(Movement::entered_freefall) && !flags.movement.test(Movement::freefalling) && !flags.movement.test(Movement::is_wall_sliding)) {
		behavior.fall();
		flags.movement.set(Movement::freefalling);
		flags.movement.reset(Movement::entered_freefall);
	}

	if (behavior.current_state.params.behavior_id == "suspended") {
		if (grounded()) { behavior.reset(); }
	}

	if (collider.flags.test(shape::State::just_landed) && jump_request == -1) {
		behavior.land();
		flags.sounds.set(Soundboard::land);
		flags.movement.reset(Movement::freefalling);
	}

	if (just_hurt) {
		behavior.hurt();
		just_hurt = false;
	}

	if (flags.movement.test(Movement::wall_slide_trigger)) { flags.movement.set(Movement::is_wall_sliding); }
	if (weapon_fired) { start_cooldown = true; }

	flags.movement.reset(Movement::stopping);
	flags.movement.reset(Movement::just_stopped);
	collider.flags.reset(shape::State::just_landed);
	flags.movement.reset(Movement::left_released);
	flags.movement.reset(Movement::right_released);
	flags.movement.reset(Movement::wall_slide_trigger);
	flags.movement.reset(Movement::release_wallslide);
	if (!weapons_hotbar.empty()) {
		if (!loadout.get_equipped_weapon().attributes.automatic) { weapon_fired = false; }
		if (start_cooldown) {
			loadout.get_equipped_weapon().current_cooldown--;
			if (loadout.get_equipped_weapon().current_cooldown < 0) {
				loadout.get_equipped_weapon().current_cooldown = loadout.get_equipped_weapon().attributes.cooldown_time;
				start_cooldown = false;
			}
		}
	}

	if (grounded() || (!collider.flags.test(shape::State::has_left_collision) && !collider.flags.test(shape::State::has_right_collision)) || abs(collider.physics.velocity.x) > 0.001f) { flags.movement.reset(Movement::is_wall_sliding); }
	update_direction();
	if (!weapons_hotbar.empty()) { update_weapon(); }
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
	behavior.facing = last_dir;
	behavior.facing_und = behavior::DIR_UND::NEUTRAL;
	if (behavior.facing_right()) {
		behavior.facing = behavior::DIR::RIGHT;
		if (flags.movement.test(Movement::look_up)) {
			behavior.facing = behavior::DIR::UP_RIGHT;
			behavior.facing_und = behavior::DIR_UND::UP;
		}
		if (flags.movement.test(Movement::look_down) && !grounded()) {
			behavior.facing = behavior::DIR::DOWN_RIGHT;
			behavior.facing_und = behavior::DIR_UND::DOWN;
		}
	}
	if (behavior.facing_left()) {
		behavior.facing = behavior::DIR::LEFT;
		if (flags.movement.test(Movement::look_up)) {
			behavior.facing = behavior::DIR::UP_LEFT;
			behavior.facing_und = behavior::DIR_UND::UP;
		}
		if (flags.movement.test(Movement::look_down) && !grounded()) {
			behavior.facing = behavior::DIR::DOWN_LEFT;
			behavior.facing_und = behavior::DIR_UND::DOWN;
		}
	}
	if (!moving() && flags.movement.test(Movement::look_up)) {
		if (behavior.facing_strictly_left()) {
			behavior.facing = behavior::DIR::UP_LEFT;
			behavior.facing_und = behavior::DIR_UND::UP;
		} else {
			behavior.facing = behavior::DIR::UP_RIGHT;
			behavior.facing_und = behavior::DIR_UND::UP;
		}
	}
	if (!flags.movement.test(Movement::move_left) && !flags.movement.test(Movement::move_right) && flags.movement.test(Movement::look_down) && !grounded()) {
		if (behavior.facing_strictly_left()) {
			behavior.facing = behavior::DIR::DOWN_LEFT;
			behavior.facing_und = behavior::DIR_UND::DOWN;
		} else {
			behavior.facing = behavior::DIR::DOWN_RIGHT;
			behavior.facing_und = behavior::DIR_UND::DOWN;
		}
	}
	if (behavior.facing_left()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2 - ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	} else if (behavior.facing_right()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2 + ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	} else {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	}
}

void Player::update_weapon() {
	switch (behavior.facing_lr) {
	case behavior::DIR_LR::LEFT: collider.physics.dir = components::DIRECTION::LEFT; break;
	case behavior::DIR_LR::RIGHT: collider.physics.dir = components::DIRECTION::RIGHT; break;
	}
	switch (behavior.facing) {
	case behavior::DIR::NEUTRAL: break;
	case behavior::DIR::LEFT: break;
	case behavior::DIR::RIGHT: break;
	case behavior::DIR::UP: collider.physics.dir = components::DIRECTION::UP; break;
	case behavior::DIR::DOWN: collider.physics.dir = components::DIRECTION::DOWN; break;
	case behavior::DIR::UP_RIGHT: collider.physics.dir = components::DIRECTION::UP; break;
	case behavior::DIR::UP_LEFT: collider.physics.dir = components::DIRECTION::UP; break;
	case behavior::DIR::DOWN_RIGHT: collider.physics.dir = components::DIRECTION::DOWN; break;
	case behavior::DIR::DOWN_LEFT: collider.physics.dir = components::DIRECTION::DOWN; break;
	}

	loadout.get_equipped_weapon().update();
	loadout.get_equipped_weapon().set_orientation();
	if (behavior.facing_right()) {
		hand_position = {28, 36};
	} else {
		hand_position = {20, 36};
	}
}

void Player::walk() {
	if (flags.movement.test(Movement::move_right) && !collider.flags.test(shape::State::has_right_collision)) { collider.physics.acceleration.x = grounded() ? stats.X_ACC : (stats.X_ACC_AIR / stats.AIR_MULTIPLIER); }
	if (flags.movement.test(Movement::move_left) && !collider.flags.test(shape::State::has_left_collision)) { collider.physics.acceleration.x = grounded() ? -stats.X_ACC : (-stats.X_ACC_AIR / stats.AIR_MULTIPLIER); }
	if (behavior.current_state.get_frame() == 44 || behavior.current_state.get_frame() == 46) {
		if (behavior.current_state.params.frame_trigger) { flags.sounds.set(Soundboard::step); }
	}
}

void Player::autonomous_walk() {
	collider.physics.acceleration.x = grounded() ? stats.X_ACC : (stats.X_ACC_AIR / stats.AIR_MULTIPLIER);
	if (behavior.facing_lr == behavior::DIR_LR::LEFT) { collider.physics.acceleration.x *= -1.f; }
	flags.movement.set(Movement::autonomous_walk);
	if (behavior.current_state.get_frame() == 44 || behavior.current_state.get_frame() == 46) {
		if (behavior.current_state.params.frame_trigger) { flags.sounds.set(Soundboard::step); }
	}
}

void Player::hurt(int amount = 1) {

	if (!is_invincible()) {
		player_stats.health -= amount;
		collider.physics.acceleration.y = -stats.HURT_ACC;
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
		a.collider.handle_collider_collision(head.bounding_box);
		a.collider.handle_collider_collision(collider.bounding_box);
		if (behavior.facing_lr == behavior::DIR_LR::RIGHT) {
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
	weapon_fired = false;
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
}

bool Player::grounded() const { return collider.flags.test(shape::State::grounded); }

bool Player::moving() const { return flags.movement.test(Movement::move_left) || flags.movement.test(Movement::move_right) || flags.movement.test(Movement::autonomous_walk); }

bool Player::moving_at_all() const {
	return flags.movement.test(Movement::move_left) || flags.movement.test(Movement::move_right) || flags.movement.test(Movement::autonomous_walk) || flags.movement.test(Movement::freefalling) ||
		   flags.movement.test(Movement::entered_freefall);
}

sf::Vector2<float> Player::get_fire_point() {
	if (behavior.facing_strictly_left()) {
		return apparent_position + hand_position + sf::Vector2<float>{static_cast<float>(-loadout.get_equipped_weapon().sprite_dimensions.x), 0.0f} - sf::Vector2<float>{asset::NANI_SPRITE_WIDTH / 2, asset::NANI_SPRITE_WIDTH / 2};
	} else if (behavior.facing_strictly_right()) {
		return apparent_position + hand_position + sf::Vector2<float>{static_cast<float>(loadout.get_equipped_weapon().sprite_dimensions.x), 0.0f} - sf::Vector2<float>{asset::NANI_SPRITE_WIDTH / 2, asset::NANI_SPRITE_WIDTH / 2};
	} else if (behavior.facing_up()) {
		return apparent_position + sf::Vector2<float>{PLAYER_WIDTH / 2, 0.0f} - sf::Vector2<float>{asset::NANI_SPRITE_WIDTH / 2, asset::NANI_SPRITE_WIDTH / 2};
	} else {
		return apparent_position + sf::Vector2<float>{PLAYER_WIDTH / 2, PLAYER_HEIGHT} - sf::Vector2<float>{asset::NANI_SPRITE_WIDTH / 2, asset::NANI_SPRITE_WIDTH / 2};
	}
}

void Player::make_invincible() { counters.invincibility = INVINCIBILITY_TIME; }

void Player::update_invincibility() {

	if (is_invincible()) {
		flash_sprite();
	} else {
		sprite.setColor(sf::Color::White);
	}

	dt = svc::clockLocator.get().tick_rate;

	auto new_time = Clock::now();
	Time frame_time = std::chrono::duration_cast<Time>(new_time - current_time);

	if (frame_time.count() > svc::clockLocator.get().frame_limit) { frame_time = Time{svc::clockLocator.get().frame_limit}; }
	current_time = new_time;
	accumulator += frame_time;

	int integrations = 0;
	if (accumulator >= dt) {

		--counters.invincibility;
		if (counters.invincibility < 0) { counters.invincibility = 0; }

		accumulator = Time::zero();
		++integrations;
	}
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

behavior::DIR_LR Player::entered_from() { return (collider.physics.position.x < lookup::SPACING * 8) ? behavior::DIR_LR::RIGHT : behavior::DIR_LR::LEFT; }

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
		switch (behavior.facing_lr) {
		case behavior::DIR_LR::LEFT: return "LEFT"; break;
		case behavior::DIR_LR::RIGHT: return "RIGHT"; break;
		}
	}
	switch (behavior.facing_und) {
	case behavior::DIR_UND::NEUTRAL: return "NEUTRAL"; break;
	case behavior::DIR_UND::UP: return "UP"; break;
	case behavior::DIR_UND::DOWN: return "DOWN"; break;
	}
}

} // namespace player