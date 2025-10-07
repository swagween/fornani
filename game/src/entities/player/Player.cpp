
#include "fornani/entities/player/Player.hpp"
#include <fornani/gui/console/Console.hpp>
#include <fornani/utils/Random.hpp>
#include "fornani/entities/item/Drop.hpp"
#include "fornani/gui/InventoryWindow.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::player {

constexpr auto wallslide_threshold_v = -0.16f;
constexpr auto walljump_force_v = 8.6f;
constexpr auto light_offset_v = 12.f;

Player::Player(automa::ServiceProvider& svc)
	: arsenal(svc), m_services(&svc), controller(svc, *this), animation(*this), sprite{svc.assets.get_texture("nani")}, wardrobe_widget(svc), m_sprite_dimensions{24, 24}, dash_effect{16},
	  m_directions{.desired{LR::left}, .actual{LR::right}}, health_indicator{svc}, orb_indicator{svc, graphics::IndicatorType::orb}, collider{player_dimensions_v}, m_sprite_shake{40}, m_hurt_cooldown{64} {
	sprite.setScale(constants::f_scale_vec);
	svc.data.load_player_params(*this);

	anchor_point = collider.physics.position + player_dimensions_v * 0.5f;
	collider.collision_depths = util::CollisionDepth();
	health.set_invincibility(300);
	hurtbox.set_dimensions(sf::Vector2f{12.f, 26.f});

	collider.physics = components::PhysicsComponent({physics_stats.ground_fric, physics_stats.ground_fric}, physics_stats.mass);
	collider.physics.maximum_velocity = physics_stats.maximum_velocity;
	collider.flags.general.set(shape::General::complex);

	antennae.push_back(vfx::Gravitator(collider.physics.position, colors::bright_orange, antenna_force));
	antennae.push_back(vfx::Gravitator(collider.physics.position, colors::bright_orange, antenna_force, {2.f, 4.f}));

	antennae[0].collider.physics = components::PhysicsComponent(sf::Vector2f{physics_stats.antenna_friction, physics_stats.antenna_friction}, 1.0f);
	antennae[0].collider.physics.maximum_velocity = sf::Vector2f(antenna_speed, antenna_speed);
	antennae[1].collider.physics = components::PhysicsComponent(sf::Vector2f{physics_stats.antenna_friction, physics_stats.antenna_friction}, 1.0f);
	antennae[1].collider.physics.maximum_velocity = sf::Vector2f(antenna_speed, antenna_speed);

	texture_updater.load_base_texture(svc.assets.get_texture_modifiable("nani"));
	texture_updater.load_pixel_map(svc.assets.get_texture_modifiable("nani_palette_default"));
	catalog.wardrobe.set_palette(svc.assets.get_texture_modifiable("nani_palette_default"));

	m_lighting.physics.velocity = random::random_vector_float(-1.f, 1.f);
	m_lighting.physics.set_global_friction(0.95f);
	m_lighting.physics.position = collider.get_center();

	distant_vicinity.set_dimensions({256.f, 256.f});
}

void Player::update(world::Map& map) {
	caution.avoid_ledges(map, collider, controller.direction, 8);
	if (collider.collision_depths) { collider.collision_depths.value().reset(); }
	collider.set_direction(Direction{m_directions.actual});
	cooldowns.tutorial.update();
	if (m_hurt_cooldown.is_almost_complete()) { m_services->music_player.filter_fade_out(); }
	m_hurt_cooldown.update();
	distant_vicinity.set_position(collider.get_center() - distant_vicinity.get_dimensions() * 0.5f);

	// map effects
	if (controller.is_wallsliding()) {
		auto freq = controller.wallslide_slowdown.get_quadratic_normalized() * 80.f;
		if (m_services->ticker.every_x_ticks(std::clamp(static_cast<int>(freq), 24, 80))) {
			map.effects.push_back(entity::Effect(*m_services, "wallslide", collider.get_center() + sf::Vector2f{12.f * controller.direction.as_float(), -8.f}, collider.physics.apparent_velocity() * 0.3f));
		}
	}
	if (controller.is_rolling() || animation.is_state(AnimState::turn_slide)) {
		if (m_services->ticker.every_x_ticks(24)) { map.effects.push_back(entity::Effect(*m_services, "roll", collider.get_center(), sf::Vector2f{collider.physics.apparent_velocity().x * 0.1f, 0.f})); }
	}
	animation.is_state(AnimState::turn_slide) && animation.animation.get_frame_count() > 4 ? m_services->soundboard.flags.player.set(audio::Player::turn_slide) : m_services->soundboard.flags.player.reset(audio::Player::turn_slide);

	// camera stuff
	auto camx = controller.direction.as_float() * 32.f;
	auto skew = 180.f;
	m_camera.target_point = sf::Vector2f{camx, skew * controller.vertical_movement()};
	auto force_multiplier = 1.f;
	if (controller.is_dashing() || controller.sprint_held()) {
		force_multiplier = 1.f;
		m_camera.target_point = sf::Vector2f{camx, 0.f};
	}
	if (!is_dead() && m_services->camera_controller.is_owned_by(graphics::CameraOwner::player)) { m_camera.camera.center(get_camera_focus_point(), force_multiplier); }
	if (m_services->camera_controller.is_owned_by(graphics::CameraOwner::system)) { m_camera.camera.center(m_services->camera_controller.get_position()); }
	m_camera.camera.update(*m_services);

	invincible() ? collider.draw_hurtbox.setFillColor(colors::red) : collider.draw_hurtbox.setFillColor(colors::blue);
	if (flags.state.test(State::crushed)) { collider.physics.gravity = 0.f; }

	collider.physics.set_constant_friction({physics_stats.ground_fric, physics_stats.air_fric});
	collider.physics.gravity = physics_stats.grav;

	update_direction();

	controller.update(*m_services, map, *this);
	if (collider.hit_ceiling_ramp()) { controller.flush_ability(); }
	controller.is_crouching() ? collider.flags.movement.set(shape::Movement::crouching) : collider.flags.movement.reset(shape::Movement::crouching);

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

	// lighting
	auto light_target = collider.get_center() + sf::Vector2f{controller.direction.as_float() * light_offset_v, 0.f};
	m_lighting.steering.seek(m_lighting.physics, light_target, 0.0052f);
	m_lighting.steering.smooth_random_walk(m_lighting.physics, 0.0041f, 64.f);
	m_lighting.physics.simple_update();

	// check direction switch
	m_directions.desired = controller.last_requested_direction();
	controller.direction.lnr = m_directions.desired.as<LNR>();

	if (hotbar) { hotbar.value().switch_weapon(*m_services, static_cast<int>(controller.arms_switch())); }
	update_animation();
	update_sprite();
	walk();
	if (!controller.moving() && (!force_cooldown.running() || collider.world_grounded())) { collider.physics.acceleration.x = 0.0f; }

	// weapon
	if (controller.is(AbilityType::walljump) && controller.is_ability_active()) { accumulated_forces.push_back({walljump_force_v * controller.get_ability_direction().as_float(), 0.f}); }
	if (controller.shot() || controller.arms_switch()) { animation.idle_timer.start(); }
	if (flags.state.test(State::impart_recoil) && arsenal) {
		if (controller.direction.und == UND::down) { accumulated_forces.push_back({0.f, -equipped_weapon().get_recoil()}); }
		if (controller.direction.und == UND::up) { accumulated_forces.push_back({0.f, equipped_weapon().get_recoil()}); }
		flags.state.reset(State::impart_recoil);
	}
	auto crouch_offset = sf::Vector2f{controller.direction.as_float() * 12.f, 10.f};
	m_weapon_socket = animation.is_state(AnimState::crawl) || animation.is_state(AnimState::crouch) ? collider.get_center() + crouch_offset : collider.get_center();

	force_cooldown.update();
	for (auto& force : accumulated_forces) { collider.physics.apply_force(force); }
	accumulated_forces.clear();
	collider.physics.impart_momentum();
	if (controller.moving() || collider.has_horizontal_collision() || collider.flags.external_state.test(shape::ExternalState::vert_world_collision) || collider.world_grounded()) { collider.physics.forced_momentum = {}; }
	auto switched = directions.movement.lnr != controller.direction.lnr || !controller.moving();
	if (collider.has_horizontal_collision() || collider.flags.external_state.test(shape::ExternalState::vert_world_collision) || collider.world_grounded() || switched || grounded()) { collider.physics.forced_acceleration = {}; }

	collider.update(*m_services);
	hurtbox.set_position(collider.hurtbox.get_position() - sf::Vector2f{0.f, 10.f});
	health.update();
	health_indicator.update(*m_services, collider.physics.position);
	orb_indicator.update(*m_services, collider.physics.position);
	if (orb_indicator.active()) { health_indicator.shift(); }
	update_invincibility();
	update_weapon();
	if (controller.is_dashing() && m_services->ticker.every_x_ticks(8)) {
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.get_center() - sf::Vector2f{0.f, 4.f}, sf::Vector2f{8.f, 8.f}, "dash", colors::ui_white, get_actual_direction()));
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
	calculate_sprite_offset();
	if (flags.state.test(State::crushed)) { return; }
	// debug
	collider.colors.local = can_jump() ? colors::green : colors::green;

	// piggybacker
	if (piggybacker) { piggybacker.value().render(svc, win, cam); }

	sprite.setPosition(sprite_position);

	// get UV coords
	static auto shake_offset = sf::Vector2f{};
	if (m_sprite_shake.running()) {
		auto shake_me = m_sprite_shake.get() % 4 == 0;
		shake_offset = shake_me ? random::random_vector_float(-4.f, 4.f) : shake_offset;
	} else {
		shake_offset = {};
	}
	auto frames_per_col = 10;
	auto u = (animation.get_frame() / frames_per_col) * m_sprite_dimensions.x;
	auto v = (animation.get_frame() % frames_per_col) * m_sprite_dimensions.y;
	sprite.setTextureRect(sf::IntRect({u, v}, m_sprite_dimensions));
	sprite.setOrigin(sprite.getLocalBounds().getCenter());
	sprite.setPosition(sprite_position - cam + shake_offset);

	if (arsenal && hotbar) { collider.flags.general.set(shape::General::complex); }

	if (svc.greyblock_mode()) {
		win.draw(sprite);
		sf::RectangleShape box{};
		box.setFillColor(sf::Color::Transparent);
		box.setOutlineColor(colors::green);
		box.setOutlineThickness(-1);
		box.setPosition(hurtbox.get_position() - cam);
		box.setSize(hurtbox.get_dimensions());
		win.draw(box);
		box.setFillColor(sf::Color::Transparent);
		box.setOutlineColor(colors::bright_purple);
		box.setOutlineThickness(-1);
		box.setPosition(distant_vicinity.get_position() - cam);
		box.setSize(distant_vicinity.get_dimensions());
		win.draw(box);
		// camera control debug
		sf::RectangleShape camera_target{};
		camera_target.setFillColor(colors::pioneer_red);
		camera_target.setSize({4.f, 4.f});
		camera_target.setOrigin({2.f, 2.f});
		camera_target.setPosition(m_camera.target_point + collider.get_center() - cam);
		win.draw(camera_target);
		camera_target.setFillColor(colors::green);
		camera_target.setPosition(svc.window->f_center_screen());
		win.draw(camera_target);
		collider.render(win, cam);
	} else {
		antennae[1].render(svc, win, cam, 1);
		win.draw(sprite);
		antennae[0].render(svc, win, cam, 1);
	}

	if (arsenal && hotbar) {
		if (flags.state.test(State::show_weapon)) { equipped_weapon().render(svc, win, cam); }
	}

	// light debug
	auto lightpos = sf::RectangleShape{{2.f, 2.f}};
	lightpos.setOrigin({1.f, 1.f});
	lightpos.setFillColor(colors::pioneer_red);
	lightpos.setPosition(get_lantern_position() - cam);
	// win.draw(lightpos);
}

void Player::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f forced_position) {
	auto frames_per_col = 10;
	auto u = (animation.get_frame() / frames_per_col) * m_sprite_dimensions.x;
	auto v = (animation.get_frame() % frames_per_col) * m_sprite_dimensions.y;
	sprite.setTextureRect(sf::IntRect({u, v}, m_sprite_dimensions));
	sprite.setOrigin(sprite.getLocalBounds().getCenter());
	sprite.setPosition(forced_position - cam);
	win.draw(sprite);
}

void Player::render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	health_indicator.render(svc, win, cam);
	orb_indicator.render(svc, win, cam);
}

void Player::assign_texture(sf::Texture& tex) { sprite.setTexture(tex); }

void Player::start_tick() {
	collider.flags.external_state.reset(shape::ExternalState::grounded);
	controller.stop_walking_autonomously();
}

void Player::end_tick() {
	controller.clean();
	flags.triggers = {};
}

void Player::update_animation() {

	m_sprite_shake.update();
	flags.state.set(State::show_weapon);

	if (grounded()) {
		if (controller.inspecting()) { animation.request(AnimState::inspect); }
		if (!(animation.is_state(AnimState::land) || animation.is_state(AnimState::rise))) {
			if (controller.nothing_pressed() && !controller.is_dashing() && !(animation.is_state(AnimState::inspect)) && !(animation.is_state(AnimState::sit))) { animation.request(AnimState::idle); }
			if (controller.moving() && !controller.is_dashing() && !controller.sprinting()) { animation.request(AnimState::run); }
			if (controller.moving() && controller.sprinting() && !controller.is_dashing()) { animation.request(AnimState::sprint); }
			if (abs(collider.physics.velocity.x) > thresholds.stop && !controller.moving()) { animation.request(AnimState::stop); }
			if (hotbar && arsenal) {
				if (controller.shot() && equipped_weapon().can_shoot()) { animation.request(AnimState::shoot); }
			}
			handle_turning();
		}
	} else {
		if (collider.physics.apparent_velocity().y > -thresholds.suspend && collider.physics.apparent_velocity().y < thresholds.suspend && !controller.is_wallsliding() && !controller.is_walljumping()) {
			animation.request(AnimState::suspend);
		}
	}

	if (collider.physics.apparent_velocity().y > thresholds.suspend && !grounded()) { animation.request(AnimState::fall); }

	if (controller.get_ability_animation() && controller.is_ability_active() && controller.is_animation_request()) { animation.request(*controller.get_ability_animation()); }

	if (animation.is_state(AnimState::sit)) { flags.state.reset(State::show_weapon); }
	if (controller.inspecting()) { animation.request(AnimState::inspect); }
	if (controller.is_crouching() && grounded()) { controller.moving() ? animation.request(AnimState::crawl) : animation.request(AnimState::crouch); }
	if (controller.moving() && grounded()) {
		if (collider.has_left_wallslide_collision() && controller.horizontal_movement() < 0.f) { cooldowns.push.update(); }
		if (collider.has_right_wallslide_collision() && controller.horizontal_movement() > 0.f) { cooldowns.push.update(); }
		if (cooldowns.push.is_complete() && (collider.has_right_wallslide_collision() || collider.has_left_wallslide_collision())) { animation.request(AnimState::push); }
	}
	if (hurt_cooldown.running()) { animation.request(AnimState::hurt); }
	if (is_dead()) {
		animation.request(AnimState::die);
		flags.state.reset(State::show_weapon);
	}

	if (flags.state.consume(State::sleep)) { animation.request(player::AnimState::sleep); }
	if (flags.state.consume(State::wake_up)) {
		NANI_LOG_DEBUG(m_logger, "Wake up flag was true");
		animation.request(player::AnimState::wake_up);
	}

	animation.update();
}

void Player::update_sprite() {

	if (!grounded() || controller.is_dashing()) {
		if (m_directions.desired != m_directions.actual) { animation.triggers.set(AnimTriggers::flip); }
	}
	if (animation.triggers.consume(AnimTriggers::flip)) {
		sprite.scale({-1.f, 1.f});
		m_directions.actual.flip();
	}

	flags.state.reset(State::dir_switch);

	sprite.setTexture(texture_updater.get_dynamic_texture());
}

void Player::handle_turning() {
	if (m_directions.desired != m_directions.actual) { ccm::abs(collider.physics.velocity.x) > thresholds.quick_turn ? animation.request(AnimState::sharp_turn) : animation.request(AnimState::turn); }
}

void Player::flash_sprite() {
	auto flash_rate = 30;
	(health.invincibility.get() / flash_rate) % 2 == 0 ? sprite.setColor(colors::red) : sprite.setColor(colors::blue);
}

void Player::calculate_sprite_offset() {
	sprite_position = {collider.get_average_tick_position() + sprite_offset};
	sprite_position.x += controller.facing_left() ? -1.f : 1.f;
}

void Player::set_idle() {
	animation.force(AnimState::idle, "idle");
	animation.state_function = std::bind(&PlayerAnimation::update_idle, &animation);
}

void Player::set_sleeping() {
	animation.force(AnimState::idle, "sleep");
	animation.state_function = std::bind(&PlayerAnimation::update_sleep, &animation);
	animation.animation.set_frame(3);
}

void Player::set_direction(Direction to) {
	m_directions.actual.set(to.lnr);
	m_directions.desired.set(to.lnr);
	controller.set_direction(to);
	sprite.setScale({constants::f_scale_vec.x * to.as_float(), constants::f_scale_vec.y});
}

void Player::piggyback(int id) { piggybacker = Piggybacker(*m_services, *m_services->data.get_npc_label_from_id(id), collider.physics.position); }

void Player::set_position(sf::Vector2f new_pos, bool centered) {
	sf::Vector2f offset{};
	offset.x = centered ? collider.dimensions.x * 0.5f : 0.f;
	collider.physics.position = new_pos - offset;
	collider.sync_components();
	update_direction();
	sync_antennae();
	health_indicator.set_position(new_pos);
	orb_indicator.set_position(new_pos);
	m_lighting.physics.position = collider.get_center() + sf::Vector2f{controller.direction.as_float() * light_offset_v, 0.f};
	if (arsenal && hotbar) {
		equipped_weapon().update(*m_services, controller.direction);
		equipped_weapon().force_position(m_weapon_socket);
	}
}

void Player::freeze_position() {
	set_position(collider.physics.previous_position);
	collider.physics.zero();
}

void Player::shake_sprite() { m_sprite_shake.start(); }

void Player::update_direction() {
	directions.movement.lnr = collider.physics.apparent_velocity().x > 0.f ? LNR::right : LNR::left;
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
		hotbar->has(weapon->get_tag()) ? weapon->set_hotbar() : weapon->set_reserved();
		weapon->set_firing_direction(controller.direction);
		if (controller.is_wallsliding()) { weapon->get_firing_direction().flip(); }
		weapon->update(*m_services, controller.direction);
		weapon->set_position(m_weapon_socket);
	}
}

void Player::walk() {
	if (animation.is_state(AnimState::slide)) { return; }
	if (animation.is_state(AnimState::dash)) { return; }
	if (animation.is_state(AnimState::sharp_turn)) { collider.physics.acceleration.x *= 0.1f; }
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
		m_services->music_player.filter_fade_in(80.f, 40.f, 32);
		m_services->ambience_player.set_balance(1.f);
		m_services->ticker.freeze_frame(12 * std::min(static_cast<int>(amount), 3));
		m_sprite_shake.start();
		m_hurt_cooldown.start();
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
		directions.left_squish.und = collider.horizontal_squish() ? UND::up : UND::neutral;
		directions.left_squish.lnr = collider.vertical_squish() ? LNR::left : LNR::neutral;
		directions.right_squish.und = collider.horizontal_squish() ? UND::down : UND::neutral;
		directions.right_squish.lnr = collider.vertical_squish() ? LNR::right : LNR::neutral;
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.physics.position, collider.dimensions, "player_crush", colors::nani_white, directions.left_squish));
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.physics.position, collider.dimensions, "player_crush", colors::nani_white, directions.right_squish));
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
		} else if (controller.is_crouching()) {
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

bool Player::grounded() const { return collider.flags.external_state.test(shape::ExternalState::grounded); }

bool Player::fire_weapon() {
	if (!arsenal || !hotbar) { return false; }
	if (controller.shot() && equipped_weapon().can_shoot()) {
		m_services->soundboard.flags.weapon.set(static_cast<audio::Weapon>(equipped_weapon().get_sound_id()));
		flags.state.set(State::impart_recoil);
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
	if (is_dead()) { return; }
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
		case 0: give_item("rhenite", 1); break;
		case 1: give_item("sapphire", 1); break;
		case 2: give_item("chalcedony", 1); break;
		}
	}
}

void Player::give_item_by_id(int id, int amount) {
	give_item(m_services->data.item_label_from_id(id), amount);
	m_services->events.dispatch_event("AcquireItem", id);
}

void Player::add_to_hotbar(std::string_view tag) {
	if (hotbar) {
		hotbar.value().add(tag);
	} else {
		hotbar = arms::Hotbar(1);
		hotbar.value().add(tag);
	}
}

void Player::remove_from_hotbar(std::string_view tag) {
	if (hotbar) {
		hotbar.value().remove(tag);
		if (hotbar.value().size() < 1) { hotbar = {}; }
	}
}

void Player::set_outfit(std::array<int, static_cast<int>(ApparelType::END)> to_outfit) {
	for (auto i{0}; i < to_outfit.size(); ++i) { catalog.wardrobe.equip(static_cast<ApparelType>(i), to_outfit[i]); }
}

void Player::give_item(std::string_view label, int amount, bool from_save) {
	auto id{0};
	for (auto i{0}; i < amount; ++i) { id = catalog.inventory.add_item(m_services->data.item, label); }
	if (id == 29 && !from_save) {
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
	set_idle();
}

void Player::map_reset() {
	if (!m_services->app_flags.test(automa::AppFlags::custom_map_start)) { set_idle(); }
	flags.state.reset(State::killed);
	if (arsenal) { arsenal.value().reset(); }
	health.invincibility.cancel();
	controller.flush_ability();
}

arms::Weapon& Player::equipped_weapon() { return arsenal.value().get_weapon_at(hotbar.value().get_tag()); }

void Player::push_to_loadout(std::string_view tag, bool from_save) {
	if (!arsenal) { arsenal = arms::Arsenal(*m_services); }
	if (!hotbar && !from_save) { hotbar = arms::Hotbar(1); }
	if (tag == "bryns_gun" && !from_save) {
		m_services->stats.time_trials.bryns_gun = m_services->ticker.in_game_seconds_passed.count();
		auto bg = util::QuestKey{1, 111, 1};
		m_services->quest.process(*m_services, bg);
	}
	if (tag == "gnat" && !from_save) { m_services->data.switch_destructible_state(103); }
	arsenal.value().push_to_loadout(tag);
	if (!from_save) { hotbar.value().add(tag); }
	m_services->stats.player.guns_collected.update();
}

void Player::pop_from_loadout(std::string_view tag) {
	if (!arsenal) { throw std::runtime_error("Cannot pop weapon from empty Arsenal."); }
	if (!hotbar) { throw std::runtime_error("Cannot pop weapon from empty Hotbar."); }
	arsenal.value().pop_from_loadout(tag);
	hotbar.value().remove(tag);
	if (arsenal.value().empty()) {
		arsenal = {};
		hotbar = {};
	}
}

SimpleDirection Player::entered_from() const { return (collider.physics.position.x < constants::f_cell_size * 8.f) ? SimpleDirection(LR::right) : SimpleDirection(LR::left); }

bool Player::can_dash() const {
	if (health.is_dead()) { return false; }
	if (grounded()) { return false; }
	if (!catalog.inventory.has_item("old_ivory_amulet")) { return false; }
	if (m_ability_usage.dash.get_count() > 0) {
		if (catalog.inventory.has_item("imbier_totem") && m_ability_usage.dash.get_count() < 2) { return true; }
		return false;
	}
	return true;
}

bool Player::can_omnidirectional_dash() const { return catalog.inventory.has_item("ancient_periapt"); }

bool Player::can_doublejump() const {
	if (health.is_dead()) { return false; }
	if (controller.is_wallsliding()) { return false; }
	if (grounded()) { return false; }
	if ((collider.has_left_wallslide_collision() || collider.has_right_wallslide_collision()) && can_wallslide()) { return false; }
	if (m_ability_usage.doublejump.get_count() > 0) { return false; }
	if (!catalog.inventory.has_item("sky_pendant")) { return false; }
	return true;
}

bool Player::can_roll() const {
	if (health.is_dead()) { return false; }
	if (controller.is_wallsliding()) { return false; }
	if (grounded()) { return false; }
	if (!catalog.inventory.has_item("woodshine_totem")) { return false; }
	return true;
}

bool Player::can_slide() const {
	if (health.is_dead()) { return false; }
	if (controller.is_wallsliding() || controller.slid_in_air()) { return false; }
	if (!grounded()) { return false; }
	if (!catalog.inventory.has_item("pioneer_medal")) { return false; }
	return true;
}

bool Player::can_jump() const {
	if (health.is_dead()) { return false; }
	if (controller.is_wallsliding()) { return false; }
	if (animation.is_state(AnimState::sleep)) { return false; }
	if (!grounded()) { return false; }
	return true;
}

bool Player::can_wallslide() const {
	if (health.is_dead()) { return false; }
	if (collider.physics.apparent_velocity().y < wallslide_threshold_v) { return false; }
	if (!catalog.inventory.has_item("kariba_talisman")) { return false; }
	return true;
}

bool Player::can_walljump() const {
	if (health.is_dead()) { return false; }
	if (!catalog.inventory.has_item("kariba_talisman")) { return false; }
	return true;
}

} // namespace fornani::player
