
#include "fornani/entities/player/Player.hpp"

#include <tracy/Tracy.hpp>

#include "fornani/entities/item/Drop.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/gui/InventoryWindow.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::player {

Player::Player(automa::ServiceProvider& svc)
	: arsenal(svc), m_services(&svc), health_indicator(svc), orb_indicator(svc), controller(svc), animation(*this), tutorial(svc), sprite{svc.assets.get_texture("nani")}, camera_offset{32.f, -64.f}, wardrobe_widget(svc),
	  m_sprite_dimensions{24, 24} {
	sprite.setScale(constants::f_scale_vec);
}

void Player::init(automa::ServiceProvider& svc) {

	m_services = &svc;

	svc.data.load_player_params(*this);
	health_indicator.init(svc, 0);
	orb_indicator.init(svc, 1);
	tutorial.update(svc);

	health.set_invincibility(400);

	collider = shape::Collider(sf::Vector2<float>{PLAYER_WIDTH, PLAYER_HEIGHT});
	hurtbox.set_dimensions(sf::Vector2<float>{12.f, 26.f});
	collider.physics = components::PhysicsComponent({physics_stats.ground_fric, physics_stats.ground_fric}, physics_stats.mass);

	collider.physics.set_constant_friction({physics_stats.ground_fric, physics_stats.air_fric});
	collider.collision_depths = util::CollisionDepth();

	anchor_point = {collider.physics.position.x + PLAYER_WIDTH / 2, collider.physics.position.y + PLAYER_HEIGHT / 2};

	antennae.push_back(vfx::Gravitator(collider.physics.position, svc.styles.colors.bright_orange, antenna_force));
	antennae.push_back(vfx::Gravitator(collider.physics.position, svc.styles.colors.bright_orange, antenna_force, {2.f, 4.f}));

	antennae[0].collider.physics = components::PhysicsComponent(sf::Vector2<float>{physics_stats.antenna_friction, physics_stats.antenna_friction}, 1.0f);
	antennae[0].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);
	antennae[1].collider.physics = components::PhysicsComponent(sf::Vector2<float>{physics_stats.antenna_friction, physics_stats.antenna_friction}, 1.0f);
	antennae[1].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);

	;

	texture_updater.load_base_texture(svc.assets.get_texture_modifiable("nani"));
	texture_updater.load_pixel_map(svc.assets.get_texture_modifiable("nani_palette_default"));
	catalog.wardrobe.set_palette(svc.assets.get_texture_modifiable("nani_palette_default"));
}

void Player::update(world::Map& map) {
	caution.avoid_ledges(map, collider, controller.direction, 8);
	if (collider.collision_depths) { collider.collision_depths.value().reset(); }
	tutorial.update(*m_services);
	cooldowns.tutorial.update();

	// camera stuff
	camera_offset.x = controller.facing_left() ? -32.f : 32.f;
	if (controller.sprinting()) { controller.reset_vertical_movement(); }
	m_camera.physics.set_global_friction(0.88f);
	auto skew{controller.vertical_movement() < 0.f ? 120.f : 160.f};
	m_camera.target.seek(m_camera.physics, sf::Vector2f{0.f, skew} * controller.vertical_movement(), 0.002f);
	m_camera.physics.simple_update();
	camera_offset.y = -64.f + m_camera.physics.position.y;

	invincible() ? collider.draw_hurtbox.setFillColor(m_services->styles.colors.red) : collider.draw_hurtbox.setFillColor(m_services->styles.colors.blue);

	collider.flags.general.set(shape::General::complex);
	if (!catalog.abilities.has_ability(Abilities::dash)) { controller.nullify_dash(); }

	collider.physics.gravity = physics_stats.grav;
	collider.physics.maximum_velocity = physics_stats.maximum_velocity;
	collider.physics.ground_friction = {physics_stats.ground_fric, physics_stats.ground_fric};
	collider.physics.air_friction = {physics_stats.air_fric, physics_stats.air_fric};
	if (flags.state.test(State::crushed)) { collider.physics.gravity = 0.f; }

	update_direction();
	grounded() ? controller.ground() : controller.unground();

	if (!catalog.abilities.has_ability(Abilities::double_jump)) { controller.get_jump().jump_counter.cancel(); }
	controller.update(*m_services);
	if (collider.hit_ceiling_ramp()) { controller.get_jump().cancel(); }

	if (grounded()) { controller.reset_dash_count(); }

	// do this elsehwere later
	if (collider.flags.state.test(shape::State::just_landed)) {
		auto below_point = collider.get_below_point();
		auto val = map.get_tile_value_at_position(below_point);
		if (val == 0) {
			below_point = collider.get_below_point(-1);
			val = map.get_tile_value_at_position(below_point);
		}
		if (val == 0) {
			below_point = collider.get_below_point(1);
			val = map.get_tile_value_at_position(below_point);
		}
		m_services->soundboard.play_step(val, map.native_style_id, true);
	}
	collider.flags.state.reset(shape::State::just_landed);

	// player-controlled actions
	if (hotbar) { hotbar.value().switch_weapon(*m_services, static_cast<int>(controller.arms_switch())); }
	dash();
	jump(map);
	wallslide();
	shield();
	update_animation();
	update_sprite();

	// check keystate
	if (!controller.get_jump().jumpsquatting()) { walk(); }
	if (!controller.moving() && (!force_cooldown.running() || collider.world_grounded())) { collider.physics.acceleration.x = 0.0f; }

	// weapon
	if (controller.shot() || controller.arms_switch()) { animation.idle_timer.start(); }
	if (flags.state.test(State::impart_recoil) && arsenal) {
		if (controller.direction.und == dir::UND::down) { accumulated_forces.push_back({0.f, -equipped_weapon().get_recoil()}); }
		if (controller.direction.und == dir::UND::up) { accumulated_forces.push_back({0.f, equipped_weapon().get_recoil()}); }
		flags.state.reset(State::impart_recoil);
	}

	force_cooldown.update();
	for (auto& force : accumulated_forces) { collider.physics.apply_force(force); }
	accumulated_forces.clear();
	collider.physics.impart_momentum();
	if (controller.moving() || collider.has_horizontal_collision() || collider.flags.external_state.test(shape::ExternalState::vert_world_collision) || collider.world_grounded()) { collider.physics.forced_momentum = {}; }
	auto switched = directions.movement.lr != controller.direction.lr || !controller.moving();
	if (collider.has_horizontal_collision() || collider.flags.external_state.test(shape::ExternalState::vert_world_collision) || collider.world_grounded() || switched || grounded()) { collider.physics.forced_acceleration = {}; }

	collider.update(*m_services);
	hurtbox.set_position(collider.hurtbox.get_position() - sf::Vector2<float>{0.f, 14.f});
	health.update();
	health_indicator.update(*m_services, collider.physics.position);
	orb_indicator.update(*m_services, collider.physics.position);
	if (orb_indicator.active()) { health_indicator.shift(); }
	update_invincibility();
	update_weapon();

	if (catalog.abilities.has_ability(Abilities::dash)) {
		if (!(animation.state == AnimState::dash) && !controller.dash_requested()) {
			controller.stop_dashing();
			controller.cancel_dash_request();
			collider.flags.dash.reset(shape::Dash::dash_cancel_collision);
		}
	}

	if (animation.state == AnimState::slide && m_services->ticker.every_x_ticks(12)) {
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.jumpbox.get_position(), collider.jumpbox.get_dimensions(), "slide", m_services->styles.colors.ui_white, dir::Direction(dir::UND::up)));
	}

	update_antennae();
	// piggybacker
	if (m_services->player_dat.piggy_id == 0 && piggybacker) { piggybacker = {}; }
	if (m_services->player_dat.piggy_id != 0 && !piggybacker) { piggyback(m_services->player_dat.piggy_id); }
	if (piggybacker) { piggybacker.value().update(*m_services, *this); }

	if (is_dead()) {
		for (auto& a : antennae) { a.collider.detect_map_collision(map); }
	}

	// step sounds
	if (m_services->in_game()) {
		if (animation.stepped() && abs(collider.physics.velocity.x) > 2.5f) { m_services->soundboard.play_step(map.get_tile_value_at_position(collider.get_below_point()), map.native_style_id); }
	}
}

void Player::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	ZoneScopedN("Player::render");
	calculate_sprite_offset();
	if (flags.state.test(State::crushed)) { return; }
	// debug
	collider.colors.local = controller.can_jump() ? svc.styles.colors.green : svc.styles.colors.green;

	// piggybacker
	if (piggybacker) { piggybacker.value().render(svc, win, cam); }

	// dashing effect
	sprite.setPosition(sprite_position);
	if (svc.ticker.every_x_frames(8) && animation.state == AnimState::dash) { sprite_history.update(sprite, collider.physics.position); }
	if (svc.ticker.every_x_frames(8) && !(animation.state == AnimState::dash)) { sprite_history.flush(); }
	sprite_history.drag(win, cam);

	// get UV coords
	auto frames_per_col = 10;
	auto u = (animation.get_frame() / frames_per_col) * m_sprite_dimensions.x;
	auto v = (animation.get_frame() % frames_per_col) * m_sprite_dimensions.y;
	sprite.setTextureRect(sf::IntRect({u, v}, m_sprite_dimensions));
	sprite.setOrigin(sprite.getLocalBounds().getCenter());
	sprite.setPosition(sprite_position - cam);

	if (arsenal && hotbar) { collider.flags.general.set(shape::General::complex); }

	if (svc.greyblock_mode()) {
		win.draw(sprite);
		collider.render(win, cam);
		sf::RectangleShape box{};
		box.setFillColor(sf::Color::Transparent);
		box.setOutlineColor(svc.styles.colors.green);
		box.setOutlineThickness(-1);
		box.setPosition(hurtbox.get_position() - cam);
		box.setSize(hurtbox.get_dimensions());
		win.draw(box);
	} else {
		antennae[1].render(svc, win, cam, 1);
		win.draw(sprite);
		antennae[0].render(svc, win, cam, 1);
	}

	if (arsenal && hotbar) {
		if (flags.state.test(State::show_weapon)) { equipped_weapon().render(svc, win, cam); }
	}

	if (controller.get_shield().active() && catalog.abilities.has_ability(Abilities::shield)) { controller.get_shield().render(*m_services, win, cam); }
}

void Player::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f forced_position) {
	ZoneScopedN("Player::render");
	auto frames_per_col = 10;
	auto u = (animation.get_frame() / frames_per_col) * m_sprite_dimensions.x;
	auto v = (animation.get_frame() % frames_per_col) * m_sprite_dimensions.y;
	sprite.setTextureRect(sf::IntRect({u, v}, m_sprite_dimensions));
	sprite.setOrigin(sprite.getLocalBounds().getCenter());
	sprite.setPosition(forced_position - cam);
	win.draw(sprite);
}

void Player::render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	health_indicator.render(svc, win, cam);
	orb_indicator.render(svc, win, cam);
}

void Player::assign_texture(sf::Texture& tex) { sprite.setTexture(tex); }

void Player::update_animation() {

	flags.state.set(State::show_weapon);

	if (grounded()) {
		if (controller.inspecting()) { animation.state = AnimState::inspect; }
		if (!(animation.state == AnimState::land || animation.state == AnimState::rise)) {
			if (controller.nothing_pressed() && !controller.dashing() && !(animation.state == AnimState::inspect) && !(animation.state == AnimState::sit)) { animation.state = AnimState::idle; }
			if (controller.moving() && !controller.dashing() && !controller.sprinting()) { animation.state = AnimState::run; }
			if (controller.moving() && controller.sprinting() && !controller.dashing() && !(animation.state == AnimState::sharp_turn)) { animation.state = AnimState::sprint; }
			if ((animation.state == AnimState::sprint || animation.state == AnimState::roll) && controller.sliding() && controller.get_slide().can_begin()) { animation.state = AnimState::slide; }
			if (abs(collider.physics.velocity.x) > thresholds.stop && !controller.moving()) { animation.state = AnimState::stop; }
			if (hotbar && arsenal) {
				if (controller.shot() && equipped_weapon().can_shoot()) { animation.state = AnimState::shoot; }
			}
			handle_turning();
		}
	} else {
		if (collider.physics.apparent_velocity().y > -thresholds.suspend && collider.physics.apparent_velocity().y < thresholds.suspend && !controller.get_wallslide().is_wallsliding() && !controller.is_walljumping()) {
			animation.state = AnimState::suspend;
		}
	}

	if (collider.physics.apparent_velocity().y > thresholds.suspend && !grounded()) { animation.state = AnimState::fall; }

	if (catalog.abilities.has_ability(Abilities::dash)) {
		if (controller.dashing() && controller.can_dash()) { animation.state = AnimState::dash; }
		if (controller.dash_requested()) {
			animation.state = AnimState::dash;
			flags.state.reset(State::show_weapon);
		}
	}
	if (catalog.abilities.has_ability(Abilities::wall_slide)) {
		if (controller.get_wallslide().is_wallsliding()) { animation.state = AnimState::wallslide; }
	}
	if (controller.moving() && grounded() && controller.can_jump()) {
		if (collider.has_left_wallslide_collision() && controller.horizontal_movement() < 0.f) { cooldowns.push.update(); }
		if (collider.has_right_wallslide_collision() && controller.horizontal_movement() > 0.f) { cooldowns.push.update(); }
		if (cooldowns.push.is_complete() && (collider.has_right_wallslide_collision() || collider.has_left_wallslide_collision())) { animation.state = AnimState::push; }
	}
	if (catalog.abilities.has_ability(Abilities::shield)) {
		if (controller.get_shield().is_shielding() && grounded() && !(animation.state == AnimState::land) && !(animation.state == AnimState::fall)) {
			animation.state = AnimState::shield;
			controller.prevent_movement();
			flags.state.reset(State::show_weapon);
		}
	}
	if (animation.state == AnimState::sit) { flags.state.reset(State::show_weapon); }
	if (hurt_cooldown.running()) { animation.state = AnimState::hurt; }
	if (controller.inspecting()) { animation.state = AnimState::inspect; }
	if (is_dead()) {
		animation.state = AnimState::die;
		flags.state.reset(State::show_weapon);
	}

	if (controller.roll.is_valid() && grounded() && controller.moving()) { animation.state = AnimState::roll; }
	// for sliding down ramps
	animation.state == AnimState::slide ? collider.flags.animation.set(shape::Animation::sliding) : collider.flags.animation.reset(shape::Animation::sliding);
	if (animation.state == AnimState::roll) { collider.flags.animation.set(shape::Animation::sliding); }

	if (animation.was_requested(AnimState::sleep)) { animation.state = player::AnimState::sleep; }
	if (animation.was_requested(AnimState::wake_up)) { animation.state = player::AnimState::wake_up; }

	animation.update();
}

void Player::update_sprite() {

	if (animation.triggers.consume(AnimTriggers::flip)) {
		sprite.scale({-1.f, 1.f});
		if (animation.animation.label == "turn" || animation.animation.label == "sharp_turn") { animation.animation.set_params(idle); }
	}

	flags.state.reset(State::dir_switch);
	// flip the sprite based on the player's direction
	if (!grounded()) {
		if (controller.facing_left() && sprite.getScale() == constants::f_scale_vec) { sprite.scale({-1.f, 1.f}); }
		if (controller.facing_right() && sprite.getScale() == constants::f_inverse_scale_vec) { sprite.scale({-1.f, 1.f}); }
	}

	// check for quick turn
	if (controller.quick_turn()) { flags.state.set(State::dir_switch); }

	sprite.setTexture(texture_updater.get_dynamic_texture());
}

void Player::handle_turning() {
	if (controller.facing_left() && sprite.getScale() == constants::f_scale_vec) { animation.state = collider.physics.velocity.x > thresholds.quick_turn ? AnimState::sharp_turn : AnimState::turn; }
	if (controller.facing_right() && sprite.getScale() == constants::f_inverse_scale_vec) { animation.state = collider.physics.velocity.x < -thresholds.quick_turn ? AnimState::sharp_turn : AnimState::turn; }
}

void Player::flash_sprite() {
	if ((health.invincibility.get_cooldown() / 30) % 2 == 0) {
		sprite.setColor(m_services->styles.colors.red);
	} else {
		sprite.setColor(m_services->styles.colors.blue);
	}
}

void Player::calculate_sprite_offset() {
	sprite_position = {collider.get_average_tick_position() + sprite_offset};
	sprite_position.x += controller.facing_left() ? -1.f : 1.f;
}

void Player::set_idle() {
	animation.state = AnimState::idle;
	animation.animation.set_params(idle);
	animation.state_function = std::bind(&PlayerAnimation::update_idle, &animation);
}

void Player::piggyback(int id) { piggybacker = Piggybacker(*m_services, m_services->tables.npc_label.at(id), collider.physics.position); }

void Player::jump(world::Map& map) {
	if (is_dead() || animation.state == AnimState::die) { return; }
	if (is_in_animation(AnimState::sleep) || is_in_animation(AnimState::wake_up)) { return; }
	if (controller.get_jump().began()) {
		collider.flags.movement.set(shape::Movement::jumping);
		animation.state = AnimState::rise;
		if (controller.get_wallslide().is_wallsliding()) { controller.walljump(); }
		if (m_services->ticker.every_x_ticks(20)) {
			map.active_emitters.push_back(vfx::Emitter(*m_services, collider.jumpbox.get_position(), collider.jumpbox.get_dimensions(), "jump", m_services->styles.colors.ui_white, dir::Direction(dir::UND::up)));
		}
		if (tutorial.current_state == text::TutorialFlags::jump) {
			tutorial.flags.set(text::TutorialFlags::jump);
			tutorial.current_state = text::TutorialFlags::sprint;
			tutorial.trigger();
			tutorial.turn_on();
		}
	} else {
		collider.flags.movement.reset(shape::Movement::jumping);
	}
	if (controller.get_jump().jumpsquat_trigger()) {
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.jumpbox.get_position(), collider.jumpbox.get_dimensions(), "jump", m_services->styles.colors.ui_white, dir::Direction(dir::UND::up)));
		controller.get_jump().start_jumpsquat();
		controller.get_jump().reset_jumpsquat_trigger();
		collider.flags.movement.set(shape::Movement::jumping);
	}
	if (controller.get_jump().jumpsquatting()) {
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.jumpbox.get_position(), collider.jumpbox.get_dimensions(), "jump", m_services->styles.colors.ui_white, dir::Direction(dir::UND::up)));
		controller.get_jump().stop_jumpsquatting();
		controller.get_jump().start();
		collider.physics.acceleration.y = -physics_stats.jump_velocity;
		collider.physics.velocity.y = 0.f;
		animation.state = AnimState::rise;
		if (controller.get_wallslide().is_wallsliding()) { controller.walljump(); }
		controller.get_wallslide().is_wallsliding() ? m_services->soundboard.flags.player.set(audio::Player::walljump) : m_services->soundboard.flags.player.set(audio::Player::jump);
		collider.flags.movement.set(shape::Movement::jumping);
	} else if (controller.get_jump().released() && controller.get_jump().jumping() && !controller.get_jump().held() && collider.physics.apparent_velocity().y < 0.0f) {
		collider.physics.acceleration.y *= physics_stats.jump_release_multiplier;
		controller.get_jump().reset();
	}
	if (collider.flags.state.test(shape::State::just_landed)) { controller.get_jump().reset_jumping(); }
	if (catalog.abilities.has_ability(Abilities::double_jump)) {
		if (controller.get_jump().just_doublejumped()) {
			collider.physics.velocity.y = 0.f;
			controller.get_jump().doublejump();
			m_services->soundboard.flags.player.set(audio::Player::jump);
			map.effects.push_back(entity::Effect(*m_services, "doublejump", sprite_position, sf::Vector2<float>{collider.physics.velocity.x * 0.1f, 0.f}, 0, 9));
		}
		if (controller.get_jump().is_doublejump()) { animation.state = AnimState::backflip; }
	}
}

void Player::dash() {
	if (!catalog.abilities.has_ability(Abilities::dash)) { return; }
	if (animation.state == AnimState::dash || controller.dash_requested()) {
		collider.flags.movement.set(shape::Movement::dashing);
		collider.physics.acceleration.y = controller.vertical_movement() * physics_stats.vertical_dash_multiplier;
		collider.physics.velocity.y = controller.vertical_movement() * physics_stats.vertical_dash_multiplier;

		if (!collider.flags.dash.test(shape::Dash::dash_cancel_collision)) {
			collider.physics.acceleration.x += controller.dash_value() * physics_stats.dash_speed;
			collider.physics.velocity.x += controller.dash_value() * physics_stats.dash_speed;
		}
		controller.dash();
	}
}

void Player::wallslide() {
	if (!catalog.abilities.has_ability(Abilities::wall_slide)) { return; }
	controller.get_wallslide().end();
	if (!grounded() && collider.physics.velocity.y > thresholds.wallslide) {
		if ((collider.has_left_wallslide_collision() && controller.moving_left()) || (collider.has_right_wallslide_collision() && controller.moving_right())) {
			controller.get_wallslide().start();
			collider.physics.acceleration.y = std::min(collider.physics.acceleration.y, physics_stats.wallslide_speed);
		}
	}
}

void Player::shield() {
	if (!catalog.abilities.has_ability(Abilities::shield)) {
		controller.get_shield().flags = {};
		return;
	}
	controller.get_shield().update(*m_services);
	controller.get_shield().sensor.bounds.setPosition(collider.bounding_box.get_position() + collider.bounding_box.get_dimensions() * 0.5f);
}

void Player::set_position(sf::Vector2<float> new_pos, bool centered) {
	sf::Vector2<float> offset{};
	offset.x = centered ? collider.dimensions.x * 0.5f : 0.f;
	collider.physics.position = new_pos - offset;
	collider.sync_components();
	update_direction();
	sync_antennae();
	health_indicator.set_position(new_pos);
	orb_indicator.set_position(new_pos);
	if (arsenal && hotbar) {
		equipped_weapon().update(*m_services, controller.direction);
		equipped_weapon().force_position(collider.get_center());
	}
}

void Player::freeze_position() {
	set_position(collider.physics.previous_position);
	collider.physics.zero();
}

void Player::update_direction() {
	directions.movement.lr = collider.physics.apparent_velocity().x > 0.f ? dir::LR::right : dir::LR::left;
	if (controller.facing_left()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.get_dimensions().x / 2 - ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.get_dimensions().y / 2};
	} else if (controller.facing_right()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.get_dimensions().x / 2 + ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.get_dimensions().y / 2};
	} else {
		anchor_point = {collider.physics.position.x + collider.bounding_box.get_dimensions().x / 2, collider.physics.position.y + collider.bounding_box.get_dimensions().y / 2};
	}
}

void Player::update_weapon() {
	controller.set_arsenal(static_cast<bool>(hotbar));
	if (!arsenal) { return; }
	if (!hotbar) { return; }
	// update all weapons in loadout to avoid unusual behavior upon fast weapon switching
	for (auto& weapon : arsenal.value().get_loadout()) {
		hotbar->has(weapon->get_id()) ? weapon->set_hotbar() : weapon->set_reserved();
		weapon->set_firing_direction(controller.direction);
		if (controller.get_wallslide().is_wallsliding()) { weapon->get_firing_direction().flip(); }
		weapon->update(*m_services, controller.direction);
		weapon->set_position(collider.get_center());
	}
}

void Player::walk() {
	if (animation.state == AnimState::slide) { return; }
	if (animation.state == AnimState::sharp_turn) { collider.physics.acceleration.x *= 0.1f; }
	if (controller.moving_right() && !collider.has_right_collision()) {
		collider.physics.acceleration.x = grounded() ? physics_stats.x_acc * controller.horizontal_movement() : (physics_stats.x_acc / physics_stats.air_multiplier) * controller.horizontal_movement();
	}
	if (controller.moving_left() && !collider.has_left_collision()) {
		collider.physics.acceleration.x = grounded() ? physics_stats.x_acc * controller.horizontal_movement() : (physics_stats.x_acc / physics_stats.air_multiplier) * controller.horizontal_movement();
	}
}

void Player::hurt(float amount, bool force) {
	if (health.is_dead()) { return; }
	if (!health.invincible() || force) {
		if (shielding() && !force) { return; }
		m_services->ticker.slow_down(25);
		health.inflict(amount, force);
		health_indicator.add(-amount);
		collider.physics.velocity.y = 0.0f;
		collider.physics.acceleration.y += -physics_stats.hurt_acc;
		force_cooldown.start(60);
		m_services->soundboard.flags.player.set(audio::Player::hurt);
		hurt_cooldown.start(2);
	}
}

void Player::on_crush(world::Map& map) {
	if (!collider.collision_depths) { return; }
	if (collider.crushed() && alive()) {
		hurt(1024.f, true);
		directions.left_squish.und = collider.horizontal_squish() ? dir::UND::up : dir::UND::neutral;
		directions.left_squish.lr = collider.vertical_squish() ? dir::LR::left : dir::LR::neutral;
		directions.right_squish.und = collider.horizontal_squish() ? dir::UND::down : dir::UND::neutral;
		directions.right_squish.lr = collider.vertical_squish() ? dir::LR::right : dir::LR::neutral;
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.physics.position, collider.dimensions, "player_crush", m_services->styles.colors.nani_white, directions.left_squish));
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.physics.position, collider.dimensions, "player_crush", m_services->styles.colors.nani_white, directions.right_squish));
		collider.collision_depths = {};
		flags.state.set(State::crushed);
	}
}

void Player::handle_map_collision(world::Map& map) { collider.detect_map_collision(map); }

void Player::update_antennae() {
	int ctr{0};
	for (auto& a : antennae) {
		if (animation.get_frame() == 44 || animation.get_frame() == 46) {
			antenna_offset.y = -19.f;
		} else if (controller.sprinting()) {
			antenna_offset.y = -13.f;
		} else if (animation.get_frame() == 52) {
			antenna_offset.y = -14.f;
		} else if (animation.get_frame() == 53) {
			antenna_offset.y = -11.f;
		} else if (animation.get_frame() == 79) {
			antenna_offset.y = 2.f;
		} else {
			antenna_offset.y = -17.f;
		}
		if (animation.get_frame() == 57) { antenna_offset.y = -8.f; }
		if (!is_dead()) {
			a.set_target_position(collider.physics.position + antenna_offset);
		} else {
			a.demagnetize(*m_services);
		}
		a.update(*m_services);
		a.collider.sync_components();
		if (controller.facing_right()) {
			antenna_offset.x = ctr % 2 == 0 ? 20.0f : 7.f;
		} else {
			antenna_offset.x = ctr % 2 == 0 ? 2.f : 13.f;
		}
		if (animation.get_frame() == 82) { antenna_offset.x += controller.facing_right() ? 6.f : -6.f; }
		++ctr;
	}
}

void Player::sync_antennae() {
	int ctr{0};
	for (auto& a : antennae) {
		a.set_position(collider.physics.position + antenna_offset);
		a.update(*m_services);
		a.collider.sync_components();
		if (controller.facing_right()) {
			antenna_offset.x = ctr % 2 == 0 ? 18.0f : 7.f;
		} else {
			antenna_offset.x = ctr % 2 == 0 ? 2.f : 13.f;
		}
		++ctr;
	}
}

bool Player::grounded() const { return collider.flags.state.test(shape::State::grounded); }

bool Player::fire_weapon() {
	if (!arsenal || !hotbar) { return false; }
	if (controller.shot() && equipped_weapon().can_shoot()) {
		m_services->soundboard.flags.weapon.set(static_cast<audio::Weapon>(equipped_weapon().get_sound_id()));
		flags.state.set(State::impart_recoil);
		if (tutorial.current_state == text::TutorialFlags::shoot) {
			tutorial.flags.set(text::TutorialFlags::shoot);
			tutorial.current_state = text::TutorialFlags::map;
			tutorial.turn_off();
		}
		return true;
	}
	return false;
}

void Player::update_invincibility() {
	hurt_cooldown.update();
	if (health.invincible()) {
		flash_sprite();
	} else {
		sprite.setColor(sf::Color::White);
	}
}

void Player::start_over() {
	flags.state.reset(State::crushed);
	health.reset();
	controller.unrestrict();
	health.invincibility.start(8);
	flags.state.reset(State::killed);
	animation.triggers.reset(AnimTriggers::end_death);
	animation.post_death.cancel();
	collider.collision_depths = util::CollisionDepth();
	for (auto& a : antennae) {
		a.collider.physics.set_global_friction(physics_stats.antenna_friction);
		a.collider.physics.gravity = 0.f;
	}
	sync_antennae();
	catalog.wardrobe.set_palette(m_services->assets.get_texture_modifiable("nani_palette_default"));
	catalog.wardrobe.update(texture_updater);
}

void Player::give_drop(item::DropType type, float value) {
	if (type == item::DropType::heart) {
		health.heal(value);
		health_indicator.add(value);
	}
	if (type == item::DropType::orb) {
		wallet.add(static_cast<int>(value));
		orb_indicator.add(value);
		m_services->stats.treasure.total_orbs_collected.update(static_cast<int>(value));
		if (value == 100) { m_services->stats.treasure.blue_orbs.update(); }
		if (orb_indicator.get_amount() > m_services->stats.treasure.highest_indicator_amount.get_count()) { m_services->stats.treasure.highest_indicator_amount.set(static_cast<int>(orb_indicator.get_amount())); }
	}
	if (type == item::DropType::gem) {
		switch (static_cast<int>(value)) {
		case 0: give_item("rhenite", item::ItemType::collectible, 1); break;
		case 1: give_item("sapphire", item::ItemType::collectible, 1); break;
		case 2: give_item("chalcedony", item::ItemType::collectible, 1); break;
		}
	}
}

void Player::add_to_hotbar(int id) {
	if (hotbar) {
		hotbar.value().add(id);
	} else {
		hotbar = arms::Hotbar(1);
		hotbar.value().add(id);
	}
}

void Player::remove_from_hotbar(int id) {
	if (hotbar) {
		hotbar.value().remove(id);
		if (hotbar.value().size() < 1) { hotbar = {}; }
	}
}

void Player::set_outfit(std::array<int, static_cast<int>(ApparelType::END)> to_outfit) {
	for (auto i{0}; i < to_outfit.size(); ++i) { catalog.wardrobe.equip(static_cast<ApparelType>(i), to_outfit[i]); }
}

void Player::give_item(std::string_view label, item::ItemType type, int amount) {
	auto id{0};
	for (auto i{0}; i < amount; ++i) { id = catalog.inventory.add_item(m_services->data.item, label, type); }
	if (id == 29) {
		health.increase_max_hp(1.f);
		m_services->soundboard.flags.item.set(audio::Item::health_increase);
	}
}

void Player::reset_flags() { flags = {}; }

void Player::total_reset() {
	start_over();
	collider.physics.zero();
	reset_flags();
	arsenal = {};
	update_antennae();
	flags.state.reset(State::killed);
	animation.state = AnimState::idle;
}

void Player::map_reset() {
	animation.state = AnimState::idle;
	flags.state.reset(State::killed);
	if (arsenal) { arsenal.value().reset(); }
}

arms::Weapon& Player::equipped_weapon() { return arsenal.value().get_weapon_at(hotbar.value().get_id()); }

void Player::push_to_loadout(int id, bool from_save) {
	if (!arsenal) { arsenal = arms::Arsenal(*m_services); }
	if (!hotbar && !from_save) { hotbar = arms::Hotbar(1); }
	if (id == 0 && !from_save) {
		m_services->stats.time_trials.bryns_gun = m_services->ticker.in_game_seconds_passed.count();
		auto bg = util::QuestKey{1, 111, 1};
		m_services->quest.process(*m_services, bg);
		tutorial.flags.set(text::TutorialFlags::inventory); // set this in case the player never opened inventory
		tutorial.current_state = text::TutorialFlags::shoot;
		tutorial.trigger();
		tutorial.turn_on();
	}
	if (id == 10 && !from_save) { m_services->quest.progress(fornani::QuestType::destroyers, 122, 1); }
	arsenal.value().push_to_loadout(id);
	if (!from_save) { hotbar.value().add(id); }
	m_services->stats.player.guns_collected.update();
}

void Player::pop_from_loadout(int id) {
	if (!arsenal) { throw std::runtime_error("Cannot pop weapon from empty Arsenal."); }
	if (!hotbar) { throw std::runtime_error("Cannot pop weapon from empty Hotbar."); }
	arsenal.value().pop_from_loadout(id);
	hotbar.value().remove(id);
	if (arsenal.value().empty()) {
		arsenal = {};
		hotbar = {};
	}
}

dir::LR Player::entered_from() const { return (collider.physics.position.x < lookup::SPACING * 8) ? dir::LR::right : dir::LR::left; }

std::string Player::print_direction(bool lr) {
	if (lr) {
		if (controller.facing_left()) return "LEFT";
		if (controller.facing_right()) return "RIGHT";
	}
	return "NULL";
}

} // namespace fornani::player
