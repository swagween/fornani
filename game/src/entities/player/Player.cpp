
#include <fornani/entities/item/Drop.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/InventoryWindow.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

constexpr auto wallslide_threshold_v = -0.16f;
constexpr auto walljump_force_v = 8.6f;
constexpr auto light_offset_v = 12.f;
constexpr auto default_invincibility_time_v = 300;

Player::Player(automa::ServiceProvider& svc)
	: Animatable(svc, "nani", {24, 24}), arsenal(svc), m_services(&svc), controller(svc, *this), m_animation_machine(*this), wardrobe_widget(svc), dash_effect{16}, m_directions{.desired{LR::right}, .actual{LR::right}},
	  health_indicator{svc}, orb_indicator{svc, graphics::IndicatorType::orb}, m_sprite_shake{40}, m_hurt_cooldown{64}, health{3.f} {

	center();
	svc.data.load_player_params(*this);

	health.set_invincibility(default_invincibility_time_v);
	hurtbox.set_dimensions(sf::Vector2f{12.f, 26.f});

	antennae.push_back(vfx::Gravitator(get_position(), colors::bright_orange, 0.62f, {2.f, 4.f}));
	antennae.push_back(vfx::Gravitator(get_position(), colors::bright_orange, 0.62f));

	texture_updater.load_base_texture(svc.assets.get_texture_modifiable("nani"));
	texture_updater.load_pixel_map(svc.assets.get_texture_modifiable("nani_palette_default"));
	catalog.wardrobe.set_palette(svc.assets.get_texture_modifiable("nani_palette_default"));

	distant_vicinity.set_dimensions({256.f, 256.f});
}

void Player::register_with_map(world::Map& map) {
	owned_collider.emplace(map, player_dimensions_v);
	collider = *owned_collider;
	if (has_collider()) { NANI_LOG_INFO(m_logger, "Player has a collider."); }
	anchor_point = get_collider().physics.position + player_dimensions_v * 0.5f;
	get_collider().collision_depths = util::CollisionDepth();
	get_collider().physics = components::PhysicsComponent({physics_stats.ground_fric, physics_stats.ground_fric}, physics_stats.mass);
	get_collider().physics.maximum_velocity = physics_stats.maximum_velocity;
	get_collider().flags.general.set(shape::General::complex);
	get_collider().set_trait(shape::ColliderTrait::player);
	get_collider().set_exclusion_target(shape::ColliderTrait::circle);
	get_collider().set_exclusion_target(shape::ColliderTrait::enemy);
	get_collider().set_exclusion_target(shape::ColliderTrait::npc);
	get_collider().set_resolution_exclusion_target(shape::ColliderTrait::platform);
	get_collider().set_resolution_exclusion_target(shape::ColliderTrait::enemy);

	m_lighting.physics.velocity = random::random_vector_float(-1.f, 1.f);
	m_lighting.physics.set_global_friction(0.95f);
	m_lighting.physics.position = get_collider().get_center();
	get_collider().clear_chunks();
}

void Player::unregister_with_map() {
	owned_collider.reset();
	collider.reset();
	NANI_LOG_INFO(m_logger, "Player was unregistered with map.");
}

void Player::update(world::Map& map) {
	tick();
	if (!collider.has_value()) { return; }
	caution.avoid_ledges(map, get_collider(), controller.direction, 8);
	if (get_collider().collision_depths) { get_collider().collision_depths.value().reset(); }
	get_collider().set_direction(Direction{m_directions.actual});
	cooldowns.tutorial.update();
	if (m_hurt_cooldown.is_almost_complete()) { m_services->music_player.filter_fade_out(); }
	m_hurt_cooldown.update();
	distant_vicinity.set_position(get_collider().get_center() - distant_vicinity.get_dimensions() * 0.5f);
	m_piggyback_socket = get_collider().get_top() + sf::Vector2f{-8.f * m_directions.actual.as_float(), -16.f};

	has_item_equipped(38) ? health.set_invincibility(default_invincibility_time_v * 1.3f) : health.set_invincibility(default_invincibility_time_v);
	if (arsenal && hotbar) { has_item_equipped(35) ? equipped_weapon().set_reload_multiplier(0.5f) : equipped_weapon().set_reload_multiplier(1.f); }

	// map effects
	if (controller.is_wallsliding()) {
		auto freq = controller.wallslide_slowdown.get_quadratic_normalized() * 80.f;
		if (m_services->ticker.every_x_ticks(std::clamp(static_cast<int>(freq), 24, 80))) {
			map.effects.push_back(entity::Effect(*m_services, "wallslide", get_collider().get_center() + sf::Vector2f{12.f * controller.direction.as_float(), -8.f}, get_collider().physics.apparent_velocity() * 0.3f));
		}
	}
	if (controller.is_rolling() || m_animation_machine.is_state(AnimState::turn_slide)) {
		if (m_services->ticker.every_x_ticks(24)) { map.effects.push_back(entity::Effect(*m_services, "roll", get_collider().get_center(), sf::Vector2f{get_collider().physics.apparent_velocity().x * 0.1f, 0.f})); }
	}
	m_animation_machine.is_state(AnimState::turn_slide) && (animation.get_frame_count() > 2 && animation.get_frame_count() < 6) ? m_services->soundboard.flags.player.set(audio::Player::turn_slide)
																																: m_services->soundboard.flags.player.reset(audio::Player::turn_slide);
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

	invincible() ? get_collider().draw_hurtbox.setFillColor(colors::red) : get_collider().draw_hurtbox.setFillColor(colors::blue);
	if (flags.state.test(State::crushed)) { get_collider().physics.gravity = 0.f; }

	get_collider().physics.set_constant_friction({physics_stats.ground_fric, physics_stats.air_fric});
	get_collider().physics.gravity = physics_stats.grav;

	update_direction();

	controller.update(*m_services, map, *this);
	if (get_collider().hit_ceiling_ramp()) { controller.flush_ability(); }
	controller.is_crouching() ? get_collider().flags.movement.set(shape::Movement::crouching) : get_collider().flags.movement.reset(shape::Movement::crouching);

	// do this elsehwere later
	if (get_collider().flags.state.test(shape::State::just_landed)) {
		auto below_point = get_collider().get_below_point();
		auto val = map.get_tile_value_at_position(below_point);
		if (val == 0) {
			below_point = get_collider().get_below_point(-1);
			val = map.get_tile_value_at_position(below_point);
		}
		if (val == 0) {
			below_point = get_collider().get_below_point(1);
			val = map.get_tile_value_at_position(below_point);
		}
		m_services->soundboard.play_step(val, map.get_style_id(), true);
	}
	get_collider().flags.state.reset(shape::State::just_landed);

	// lighting
	auto light_target = get_collider().get_center() + sf::Vector2f{controller.direction.as_float() * light_offset_v, 0.f};
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
	if (!controller.moving() && (!force_cooldown.running() || get_collider().world_grounded())) { get_collider().physics.acceleration.x = 0.0f; }

	// weapon
	if (controller.is(AbilityType::walljump) && controller.is_ability_active()) { accumulated_forces.push_back({walljump_force_v * controller.get_ability_direction().as_float(), 0.f}); }
	if (controller.shot() || controller.arms_switch()) { m_animation_machine.idle_timer.start(); }
	if (flags.state.test(State::impart_recoil) && arsenal) {
		if (controller.direction.und == UND::down) { accumulated_forces.push_back({0.f, -equipped_weapon().get_recoil()}); }
		if (controller.direction.und == UND::up) { accumulated_forces.push_back({0.f, equipped_weapon().get_recoil()}); }
		flags.state.reset(State::impart_recoil);
	}
	auto crouch_offset = sf::Vector2f{controller.direction.as_float() * 12.f, 10.f};
	m_weapon_socket = m_animation_machine.is_state(AnimState::crawl) || m_animation_machine.is_state(AnimState::crouch) ? get_collider().get_center() + crouch_offset : get_collider().get_center();

	force_cooldown.update();
	for (auto& force : accumulated_forces) { get_collider().physics.apply_force(force); }
	accumulated_forces.clear();
	get_collider().physics.impart_momentum();
	if (controller.moving() || get_collider().has_horizontal_collision() || get_collider().flags.external_state.test(shape::ExternalState::vert_world_collision) || get_collider().world_grounded()) {
		get_collider().physics.forced_momentum = {};
	}
	auto switched = directions.movement.lnr != controller.direction.lnr || !controller.moving();
	if (get_collider().has_horizontal_collision() || get_collider().flags.external_state.test(shape::ExternalState::vert_world_collision) || get_collider().world_grounded() || switched || grounded()) {
		get_collider().physics.forced_acceleration = {};
	}

	// get_collider().update(*m_services);
	hurtbox.set_position(get_collider().hurtbox.get_position() - sf::Vector2f{0.f, 10.f});
	health.update();
	health_indicator.update(*m_services, get_collider().physics.position);
	orb_indicator.update(*m_services, get_collider().physics.position);
	if (orb_indicator.active()) { health_indicator.shift(); }
	update_invincibility();
	update_weapon();
	if (controller.is_dashing() && m_services->ticker.every_x_ticks(8)) { map.spawn_emitter(*m_services, "dash", get_collider().get_center() - sf::Vector2f{0.f, 4.f}, get_actual_direction(), sf::Vector2f{8.f, 8.f}); }

	update_antennae();

	if (piggybacker) { piggybacker->update(*m_services, *this); }

	if (is_dead()) {
		for (auto& a : antennae) { a.collider.detect_map_collision(map); }
	}

	// step sounds
	if (m_services->in_game()) {
		if (m_animation_machine.stepped() && abs(get_collider().physics.velocity.x) > 2.5f) { m_services->soundboard.play_step(map.get_tile_value_at_position(get_collider().get_below_point()), map.get_style_id()); }
	}
}

void Player::simple_update() {
	handle_turning();
	tick();
	m_animation_machine.update();
	update_sprite();
	update_antennae();
	m_piggyback_socket = m_sprite_position + sf::Vector2f{-8.f * m_directions.actual.as_float(), -16.f};
	if (piggybacker) { piggybacker->update(*m_services, *this); }
}

void Player::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {

	m_sprite_position = collider.has_value() ? get_collider().get_average_tick_position() + sprite_offset : m_sprite_position;
	m_sprite_position.x += controller.facing_left() ? -1.f : 1.f;
	Animatable::set_position(m_sprite_position - cam);

	if (flags.state.test(State::crushed)) { return; }
	if (piggybacker) { piggybacker->render(svc, win, cam); }

	if (arsenal && hotbar && collider.has_value()) { get_collider().flags.general.set(shape::General::complex); }

	if (svc.greyblock_mode()) {
		win.draw(*this);
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
		if (collider.has_value()) {
			sf::RectangleShape camera_target{};
			camera_target.setFillColor(colors::pioneer_red);
			camera_target.setSize({4.f, 4.f});
			camera_target.setOrigin({2.f, 2.f});
			camera_target.setPosition(m_camera.target_point + get_collider().get_center() - cam);
			win.draw(camera_target);
			camera_target.setFillColor(colors::green);
			camera_target.setPosition(svc.window->f_center_screen());
			win.draw(camera_target);
			get_collider().render(win, cam);
		}
	} else {
		antennae[1].render(svc, win, cam, 1);
		win.draw(*this);
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
	Animatable::set_position(forced_position - cam);
	win.draw(*this);
}

void Player::render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	health_indicator.render(svc, win, cam);
	orb_indicator.render(svc, win, cam);
}

void Player::assign_texture(sf::Texture& tex) { Animatable::set_texture(tex); }

void Player::start_tick() {
	if (collider.has_value()) { get_collider().flags.external_state.reset(shape::ExternalState::grounded); }
	controller.stop_walking_autonomously();
}

void Player::end_tick() {
	controller.clean();
	flags.triggers = {};
}

void Player::update_animation() {
	if (!collider.has_value()) { return; }
	m_sprite_shake.update();
	flags.state.set(State::show_weapon);

	if (grounded()) {
		if (controller.inspecting()) { m_animation_machine.request(AnimState::inspect); }
		if (!(m_animation_machine.is_state(AnimState::land) || m_animation_machine.is_state(AnimState::rise))) {
			if (controller.nothing_pressed() && !controller.is_dashing() && !(m_animation_machine.is_state(AnimState::inspect)) && !(m_animation_machine.is_state(AnimState::sit))) { m_animation_machine.request(AnimState::idle); }
			if (controller.moving() && !controller.is_dashing() && !controller.sprinting()) { m_animation_machine.request(AnimState::run); }
			if (controller.moving() && controller.sprinting() && !controller.is_dashing()) { m_animation_machine.request(AnimState::sprint); }
			if (abs(get_collider().physics.velocity.x) > thresholds.stop && !controller.moving()) { m_animation_machine.request(AnimState::stop); }
			if (hotbar && arsenal) {
				if (controller.shot() && equipped_weapon().can_shoot()) { m_animation_machine.request(AnimState::shoot); }
			}
			handle_turning();
		}
	} else {
		if (get_collider().physics.apparent_velocity().y > -thresholds.suspend && get_collider().physics.apparent_velocity().y < thresholds.suspend && !controller.is_wallsliding() && !controller.is_walljumping()) {
			m_animation_machine.request(AnimState::suspend);
		}
	}

	if (get_collider().physics.apparent_velocity().y > thresholds.suspend && !grounded()) { m_animation_machine.request(AnimState::fall); }

	if (controller.get_ability_animation() && controller.is_ability_active() && controller.is_animation_request()) { m_animation_machine.request(*controller.get_ability_animation()); }

	if (m_animation_machine.is_state(AnimState::sit)) { flags.state.reset(State::show_weapon); }
	if (controller.inspecting()) { m_animation_machine.request(AnimState::inspect); }
	if (controller.is_crouching() && grounded()) { controller.moving() ? m_animation_machine.request(AnimState::crawl) : m_animation_machine.request(AnimState::crouch); }
	if (controller.moving() && grounded()) {
		if (get_collider().has_left_wallslide_collision() && controller.horizontal_movement() < 0.f) { cooldowns.push.update(); }
		if (get_collider().has_right_wallslide_collision() && controller.horizontal_movement() > 0.f) { cooldowns.push.update(); }
		if (cooldowns.push.is_complete() && (get_collider().has_right_wallslide_collision() || get_collider().has_left_wallslide_collision())) { m_animation_machine.request(AnimState::push); }
	}

	if (hurt_cooldown.running()) { m_animation_machine.request(AnimState::hurt); }
	if (is_dead()) {
		m_animation_machine.request(AnimState::die);
		flags.state.reset(State::show_weapon);
	}

	if (flags.state.consume(State::sleep)) { m_animation_machine.request(player::AnimState::sleep); }
	if (flags.state.consume(State::wake_up)) {
		NANI_LOG_DEBUG(m_logger, "Wake up flag was true");
		m_animation_machine.request(player::AnimState::wake_up);
	}

	m_animation_machine.update();
}

void Player::update_sprite() {

	if (has_collider()) {
		if (!grounded() || controller.is_dashing()) {
			if (m_directions.desired != m_directions.actual) { m_animation_machine.triggers.set(AnimTriggers::flip); }
		}
	}
	if (m_animation_machine.triggers.consume(AnimTriggers::flip)) {
		Animatable::scale({-1.f, 1.f});
		m_directions.actual.flip();
	}

	flags.state.reset(State::dir_switch);

	Animatable::set_texture(texture_updater.get_dynamic_texture());
}

void Player::handle_turning() {
	if (m_directions.desired != m_directions.actual) {
		if (has_collider()) {
			ccm::abs(get_collider().physics.velocity.x) > thresholds.quick_turn ? m_animation_machine.request(AnimState::sharp_turn) : m_animation_machine.request(AnimState::turn);
		} else {
			m_animation_machine.request(AnimState::turn);
		}
	}
}

void Player::flash_sprite() {
	auto flash_rate = 30;
	(health.invincibility.get() / flash_rate) % 2 == 0 ? Animatable::set_color(colors::red) : Animatable::set_color(colors::blue);
}

void Player::set_idle() {
	m_animation_machine.force(AnimState::idle, "idle");
	m_animation_machine.state_function = std::bind(&PlayerAnimation::update_idle, &m_animation_machine);
}

void Player::set_sleeping() {
	m_animation_machine.force(AnimState::idle, "sleep");
	m_animation_machine.state_function = std::bind(&PlayerAnimation::update_sleep, &m_animation_machine);
	animation.set_frame(3);
}

void Player::set_direction(Direction to) {
	m_directions.actual.set(to.lnr);
	m_directions.desired.set(to.lnr);
	controller.set_direction(to);
}

void Player::piggyback(int id) {
	if (!piggybacker) {
		piggybacker = Piggybacker(*m_services, *m_services->data.get_npc_label_from_id(id), get_center());
	} else {
		piggybacker = {};
	}
}

bool Player::is_intangible() const { return controller.is_dashing() && has_item_equipped(37); }

void Player::set_position(sf::Vector2f new_pos, bool centered) {
	sf::Vector2f offset{};
	offset.x = centered ? get_collider().dimensions.x * 0.5f : 0.f;
	m_sprite_position = new_pos;
	get_collider().physics.position = new_pos - offset;
	get_collider().sync_components();
	update_direction();
	sync_antennae();
	health_indicator.set_position(new_pos);
	orb_indicator.set_position(new_pos);
	m_lighting.physics.position = get_collider().get_center() + sf::Vector2f{controller.direction.as_float() * light_offset_v, 0.f};
	if (arsenal && hotbar) {
		equipped_weapon().update(*m_services, controller.direction);
		equipped_weapon().force_position(m_weapon_socket);
	}
}

void Player::set_draw_position(sf::Vector2f const to) {
	m_sprite_position = to;
	sync_antennae();
	health_indicator.set_position(to);
	orb_indicator.set_position(to);
	if (arsenal && hotbar) {
		equipped_weapon().update(*m_services, controller.direction);
		equipped_weapon().force_position(m_weapon_socket);
	}
}

void Player::freeze_position() {
	set_position(get_collider().physics.previous_position);
	get_collider().physics.zero();
}

void Player::shake_sprite() { m_sprite_shake.start(); }

void Player::update_direction() {
	directions.movement.lnr = get_collider().physics.apparent_velocity().x > 0.f ? LNR::right : LNR::left;
	if (controller.facing_left()) {
		anchor_point = {get_collider().physics.position.x + get_collider().bounding_box.get_dimensions().x / 2 - ANCHOR_BUFFER, get_collider().physics.position.y + get_collider().bounding_box.get_dimensions().y / 2};
	} else if (controller.facing_right()) {
		anchor_point = {get_collider().physics.position.x + get_collider().bounding_box.get_dimensions().x / 2 + ANCHOR_BUFFER, get_collider().physics.position.y + get_collider().bounding_box.get_dimensions().y / 2};
	} else {
		anchor_point = {get_collider().physics.position.x + get_collider().bounding_box.get_dimensions().x / 2, get_collider().physics.position.y + get_collider().bounding_box.get_dimensions().y / 2};
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
	if (m_animation_machine.is_state(AnimState::slide)) { return; }
	if (m_animation_machine.is_state(AnimState::dash)) { return; }
	if (m_animation_machine.is_state(AnimState::sharp_turn)) { get_collider().physics.acceleration.x *= 0.1f; }
	if (controller.moving_right() && !get_collider().has_right_collision()) {
		get_collider().physics.acceleration.x = grounded() ? physics_stats.x_acc * controller.horizontal_movement() : (physics_stats.x_acc / physics_stats.air_multiplier) * controller.horizontal_movement();
	}
	if (controller.moving_left() && !get_collider().has_left_collision()) {
		get_collider().physics.acceleration.x = grounded() ? physics_stats.x_acc * controller.horizontal_movement() : (physics_stats.x_acc / physics_stats.air_multiplier) * controller.horizontal_movement();
	}
}

void Player::hurt(float amount, bool force) {
	if (health.is_dead()) { return; }
	if (is_intangible()) { return; }
	if (!health.invincible() || force) {
		m_services->music_player.filter_fade_in(80.f, 40.f, 32);
		m_services->ambience_player.set_balance(1.f);
		m_services->ticker.freeze_frame(12 * std::min(static_cast<int>(amount), 3));
		m_sprite_shake.start();
		m_hurt_cooldown.start();
		health.inflict(amount, force);
		health_indicator.add(-amount);
		get_collider().physics.velocity.y = 0.0f;
		get_collider().physics.acceleration.y += -physics_stats.hurt_acc;
		force_cooldown.start(60);
		m_services->soundboard.flags.player.set(audio::Player::hurt);
		hurt_cooldown.start(2);
	}
}

void Player::on_crush(world::Map& map) {
	if (!get_collider().collision_depths) { return; }
	if (get_collider().crushed() && alive()) {
		hurt(1024.f, true);
		directions.left_squish.und = get_collider().horizontal_squish() ? UND::up : UND::neutral;
		directions.left_squish.lnr = get_collider().vertical_squish() ? LNR::left : LNR::neutral;
		directions.right_squish.und = get_collider().horizontal_squish() ? UND::down : UND::neutral;
		directions.right_squish.lnr = get_collider().vertical_squish() ? LNR::right : LNR::neutral;
		map.spawn_emitter(*m_services, "player_crush", get_collider().physics.position, directions.left_squish, get_collider().dimensions);
		map.spawn_emitter(*m_services, "player_crush", get_collider().physics.position, directions.right_squish, get_collider().dimensions);
		get_collider().collision_depths = {};
		flags.state.set(State::crushed);
	}
}

void Player::handle_map_collision(world::Map& map) { get_collider().detect_map_collision(map); }

void Player::update_antennae() {
	auto position = m_sprite_position;
	for (auto [i, a] : std::views::enumerate(antennae)) {
		a.attraction_force = physics_stats.antenna_force;
		a.collider.physics.set_friction_componentwise({physics_stats.antenna_friction, physics_stats.antenna_friction});
		auto& socket = i == 0 ? m_antenna_sockets.first : m_antenna_sockets.second;
		if (m_animation_machine.get_frame() == 44 || m_animation_machine.get_frame() == 46) {
			socket.y = -19.f;
		} else if (controller.sprinting()) {
			socket.y = -13.f;
		} else if (m_animation_machine.get_frame() == 52) {
			socket.y = -14.f;
		} else if (m_animation_machine.get_frame() == 53) {
			socket.y = -11.f;
		} else if (m_animation_machine.get_frame() == 79) {
			socket.y = 2.f;
		} else if (controller.is_crouching()) {
			socket.y = 2.f;
		} else {
			socket.y = -17.f;
		}
		if (m_animation_machine.get_frame() == 57) { socket.y = -8.f; }
		auto sign = m_directions.desired.as_float();
		socket.x = i == 0 ? 10.0f * sign : -3.f * sign;

		if (m_animation_machine.get_frame() == 82) { socket.x += controller.facing_right() ? 6.f : -6.f; }
		if (!is_dead()) {
			a.set_target_position(position + socket);
		} else {
			a.demagnetize(*m_services);
		}
		a.update(*m_services);
	}
}

void Player::sync_antennae() {
	auto position = m_sprite_position;
	for (auto [i, a] : std::views::enumerate(antennae)) {
		auto& socket = i == 0 ? m_antenna_sockets.first : m_antenna_sockets.second;
		a.set_position(position + socket);
		a.update(*m_services);
		if (controller.facing_right()) {
			socket.x = i == 0 ? 18.0f : 7.f;
		} else {
			socket.x = i == 0 ? 2.f : 13.f;
		}
	}
}

bool Player::grounded() const { return get_collider().flags.external_state.test(shape::ExternalState::grounded); }

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
		set_color(sf::Color::White);
	}
}

void Player::update_wardrobe() {
	catalog.wardrobe.update(texture_updater);
	wardrobe_widget.update(*this);
}

void Player::start_over() {
	flags.state.reset(State::crushed);
	health.reset();
	controller.unrestrict();
	health.invincibility.start(8);
	flags.state.reset(State::killed);
	m_animation_machine.triggers.reset(AnimTriggers::end_death);
	m_animation_machine.post_death.cancel();
	get_collider().collision_depths = util::CollisionDepth();
	for (auto& a : antennae) {
		a.collider.physics.set_global_friction(physics_stats.antenna_friction);
		a.collider.physics.gravity = 0.f;
	}
	sync_antennae();
	catalog.wardrobe.set_palette(m_services->assets.get_texture_modifiable("nani_palette_default"));
	update_wardrobe();
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

EquipmentStatus Player::equip_item(int id) { return catalog.inventory.equip_item(id); }

void Player::reset_flags() { flags = {}; }

void Player::total_reset() {
	start_over();
	get_collider().physics.zero();
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

SimpleDirection Player::entered_from() const { return (get_collider().physics.position.x < constants::f_cell_size * 8.f) ? SimpleDirection(LR::right) : SimpleDirection(LR::left); }

auto Player::can_dash_kick() const -> bool {
	if (health.is_dead()) { return false; }
	if (!catalog.inventory.has_item("forest_token")) { return false; }
	if (!flags.state.test(State::dash_kick)) { return false; }
	return true;
}

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
	if (m_animation_machine.is_state(AnimState::sleep)) { return false; }
	if (!grounded()) { return false; }
	return true;
}

bool Player::can_wallslide() const {
	if (health.is_dead()) { return false; }
	if (get_collider().physics.apparent_velocity().y < wallslide_threshold_v) { return false; }
	if (!catalog.inventory.has_item("kariba_talisman")) { return false; }
	return true;
}

bool Player::can_walljump() const {
	if (health.is_dead()) { return false; }
	if (!catalog.inventory.has_item("kariba_talisman")) { return false; }
	return true;
}

} // namespace fornani::player
