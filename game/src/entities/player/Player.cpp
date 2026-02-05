
#include <fornani/entities/item/Drop.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/events/InventoryEvent.hpp>
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
constexpr auto max_damage_v = 1024.f;

Player::Player(automa::ServiceProvider& svc)
	: Mobile(svc, "nani", {24, 24}), arsenal(svc), m_services(&svc), controller(svc, *this), m_animation_machine(*this), wardrobe_widget(svc), dash_effect{16}, health_indicator{svc}, orb_indicator{svc, graphics::IndicatorType::orb},
	  m_sprite_shake{40}, m_hurt_cooldown{64}, health{3.f}, m_air_supply{100.f}, m_air_supply_bar{svc, colors::periwinkle}, m_death_cooldown{450} {

	center();
	svc.data.load_player_params(*this);

	health.set_invincibility(default_invincibility_time_v);
	hurtbox.set_dimensions(sf::Vector2f{12.f, 26.f});

	texture_updater.load_base_texture(svc.assets.get_texture_modifiable("nani"));
	texture_updater.load_pixel_map(svc.assets.get_texture_modifiable("nani_palette_default"));
	catalog.wardrobe.set_palette(svc.assets.get_texture_modifiable("nani_palette_default"));

	distant_vicinity.set_dimensions({256.f, 256.f});

	m_ear.physics.set_global_friction(0.8f);

	svc.events.reveal_item_by_id_event.attach_to(slot, &Player::reveal_item, this);
}

void Player::serialize(dj::Json& out) const {
	out["max_hp"] = health.get_capacity();
	out["hp"] = health.get_quantity();
	out["orbs"] = wallet.get_balance();
	out["position"]["x"] = get_position().x;
	out["position"]["y"] = get_position().y;
	out["arsenal"] = dj::Json::empty_array();
	out["hotbar"] = dj::Json::empty_array();
	// push player arsenal
	if (arsenal) {
		for (auto const& gun : arsenal.value().get_loadout()) { out["arsenal"].push_back(gun->get_tag()); }
		if (hotbar) {
			for (auto const& id : hotbar.value().get_tags()) { out["hotbar"].push_back(id); }
			out["equipped_gun"] = hotbar.value().get_tag();
		}
	}

	// wardrobe
	out["wardrobe"]["hairstyle"] = static_cast<int>(catalog.wardrobe.get_variant(player::ApparelType::hairstyle));
	out["wardrobe"]["shirt"] = static_cast<int>(catalog.wardrobe.get_variant(player::ApparelType::shirt));
	out["wardrobe"]["pants"] = static_cast<int>(catalog.wardrobe.get_variant(player::ApparelType::pants));

	// items and abilities
	out["items"] = dj::Json::empty_array();
	for (auto& item : catalog.inventory.items_view()) {
		dj::Json this_item{};
		this_item["label"] = item.item->get_label();
		this_item["quantity"] = 1;
		this_item["revealed"] = item.item->is_revealed();
		out["items"].push_back(this_item);
	}

	// equipped items
	out["equipped_items"] = dj::Json::empty_array();
	for (auto const& item : catalog.inventory.equipped_items_view()) { out["equipped_items"].push_back(item); }
}

void Player::unserialize(dj::Json const& in) {
	health.set_capacity(in["max_hp"].as<float>());
	health.set_quantity(in["hp"].as<float>());
	wallet.set_balance(in["orbs"].as<int>());

	// load player's arsenal
	arsenal = {};
	hotbar = {};
	for (auto& gun_tag : in["arsenal"].as_array()) { push_to_loadout(gun_tag.as_string(), true); }
	if (!in["hotbar"].as_array().empty()) {
		if (!hotbar) { hotbar = arms::Hotbar(1); }
		if (hotbar) {
			for (auto& gun_tag : in["hotbar"].as_array()) { hotbar.value().add(gun_tag.as_string()); }
			auto equipped_gun = in["equipped_gun"].as_string();
			hotbar.value().set_selection(equipped_gun);
		}
	}

	// load items and abilities
	catalog.inventory = {};
	for (auto& item : in["items"].as_array()) {
		give_item(item["label"].as_string(), item["quantity"].as<int>(), true);
		if (item["revealed"].as_bool()) { catalog.inventory.reveal_item(m_services->data.item_id_from_label(item["label"].as_string())); }
	}

	// wardrobe
	auto& wardrobe = catalog.wardrobe;
	auto hairstyle = in["wardrobe"]["hairstyle"].as<int>();
	auto headgear = in["wardrobe"]["headgear"].as<int>();
	auto shirt = in["wardrobe"]["shirt"].as<int>();
	auto pants = in["wardrobe"]["pants"].as<int>();
	set_outfit({hairstyle, headgear, shirt, pants});
	update_wardrobe();

	// equipped items
	for (auto const& item : in["equipped_items"].as_array()) { equip_item(item.as<int>()); }
}

void Player::register_with_map(world::Map& map) {
	Mobile::register_collider(map, player_dimensions_v);

	auto result = dj::Json::from_file((m_services->finder.resource_path() + "/data/player/physics_params.json").c_str());
	if (!result) { NANI_LOG_ERROR(m_logger, "Failed to load player physics params!"); }
	auto in = std::move(*result);
	get_collider().load_properties(in["properties"]);

	if (has_collider()) { NANI_LOG_INFO(m_logger, "Player has a collider."); }
	anchor_point = get_collider().physics.position + player_dimensions_v * 0.5f;
	get_collider().collision_depths = util::CollisionDepth();
	get_collider().physics = components::PhysicsComponent({physics_stats.ground_fric, physics_stats.ground_fric}, physics_stats.mass);
	get_collider().physics.set_constant_friction({physics_stats.ground_fric, physics_stats.air_fric});
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

	collider.value().get().set_tag("Nani");
	antennae.clear();
	antennae.emplace_back(std::make_unique<vfx::Antenna>(map, get_position(), colors::bright_orange, 0.62f, sf::Vector2f{2.f, 4.f}));
	antennae.emplace_back(std::make_unique<vfx::Antenna>(map, get_position(), colors::bright_orange, 0.62f, sf::Vector2f{4.f, 4.f}));
	for (auto& a : antennae) { a->get_collider().set_attribute(shape::ColliderAttributes::no_map_collision); }
}

void Player::unregister_with_map() {
	owned_collider.reset();
	collider.reset();
	antennae.clear();
	NANI_LOG_INFO(m_logger, "Player was unregistered with map.");
}

void Player::update(world::Map& map) {
	if (is_dead() && !m_death_cooldown.running()) { m_death_cooldown.start(); }
	m_death_cooldown.update();
	if (!collider.has_value()) { return; }
	caution.avoid_ledges(map, get_collider(), controller.direction, 8);
	if (get_collider().collision_depths) { get_collider().collision_depths.value().reset(); }
	get_collider().set_direction(directions.actual);
	cooldowns.tutorial.update();
	m_hurt_cooldown.update();
	distant_vicinity.set_position(get_collider().get_center() - distant_vicinity.get_dimensions() * 0.5f);
	m_piggyback_socket = get_collider().get_top() + sf::Vector2f{-8.f * directions.actual.as_float(), -16.f};

	m_ear.seek(get_camera_focus_point(), 0.006f);

	if (get_collider().has_flag_set(shape::ColliderFlags::submerged)) {
		if (m_services->ticker.every_x_ticks(32)) { m_air_supply.inflict(1.f); }
	} else {
		if (m_services->ticker.every_x_ticks(8)) { m_air_supply.heal(1.f); }
	}

	// check for drown
	if (m_air_supply.is_dead()) {
		m_death_type = PlayerDeathType::drowned;
		hurt(max_damage_v, true);
	}
	m_air_supply_bar.update(m_air_supply.get_normalized(), get_collider().get_top() + sf::Vector2f{0.f, -32.f}, true);

	// check for fall
	if (map.off_the_bottom(get_collider().physics.position)) {
		m_death_type = PlayerDeathType::fallen;
		hurt(64.f);
		freeze_position();
	}

	if (has_item_equipped(25)) {
		if (arsenal && hotbar) {
			if (consume_flag(PlayerFlags::hit_target)) { equipped_weapon().reduce_reload_time(0.1f); }
		}
	}
	has_item_equipped(38) ? health.set_invincibility(default_invincibility_time_v * 1.3f) : health.set_invincibility(default_invincibility_time_v);
	if (arsenal && hotbar) { has_item_equipped(35) ? equipped_weapon().set_reload_multiplier(0.85f) : equipped_weapon().set_reload_multiplier(1.f); }

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
	if (m_animation_machine.is_state(AnimState::turn_slide) && (animation.get_frame_count() > 2 && animation.get_frame_count() < 6)) { m_services->soundboard.repeat_sound("nani_turn_slide"); }

	// camera stuff
	auto camx = controller.direction.as_float() * 32.f;
	auto skew = 180.f;
	auto vert = m_services->input_system.is_gamepad() ? m_services->input_system.analog(input::AnalogAction::pan).y : controller.vertical_movement();
	m_camera.target_point = sf::Vector2f{camx, skew * vert};
	auto force_multiplier = 1.f;
	if (controller.is_dashing() || controller.sprint_held()) {
		force_multiplier = 1.f;
		m_camera.target_point = sf::Vector2f{camx, 0.f};
	}
	if (!is_dead() && m_services->camera_controller.is_owned_by(graphics::CameraOwner::player)) { m_camera.camera.center(get_camera_focus_point(), force_multiplier); }
	if (m_services->camera_controller.is_owned_by(graphics::CameraOwner::system)) { m_camera.camera.center(m_services->camera_controller.get_position()); }
	m_camera.camera.update(*m_services);

	invincible() ? get_collider().draw_hurtbox.setFillColor(colors::red) : get_collider().draw_hurtbox.setFillColor(colors::blue);
	if (has_death_type(PlayerDeathType::crushed)) { get_collider().physics.gravity = 0.f; }

	get_collider().set_flag(shape::ColliderFlags::sinking, has_death_type(PlayerDeathType::drowned));

	if (get_collider().has_flag_set(shape::ColliderFlags::submerged)) {
		map.set_target_balance(0.f, audio::BalanceTarget::music);
		map.set_target_balance(0.f, audio::BalanceTarget::ambience);
	}
	if (hurt_cooldown.running()) {
		map.set_target_balance(0.f, audio::BalanceTarget::music);
		map.set_target_balance(0.f, audio::BalanceTarget::ambience);
	}

	update_direction();

	controller.update(*m_services, map, *this);
	if (get_collider().hit_ceiling_ramp()) { controller.flush_ability(); }
	controller.is_crouching() ? get_collider().flags.movement.set(shape::Movement::crouching) : get_collider().flags.movement.reset(shape::Movement::crouching);
	if (!controller.is(AbilityType::jump)) { get_collider().flags.movement.reset(shape::Movement::jumping); }

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
		if (!had_special_death()) { m_services->soundboard.play_step(val, map.get_style_id(), true); }
	}
	get_collider().flags.state.reset(shape::State::just_landed);

	// lighting
	auto light_target = get_collider().get_center() + sf::Vector2f{controller.direction.as_float() * light_offset_v, 0.f};
	m_lighting.steering.seek(m_lighting.physics, light_target, 0.0052f);
	m_lighting.steering.smooth_random_walk(m_lighting.physics, 0.0041f, 64.f);
	m_lighting.physics.simple_update();

	// check direction switch
	directions.desired = controller.last_requested_direction();
	controller.direction.lnr = directions.desired.lnr;

	if (hotbar) { hotbar.value().switch_weapon(*m_services, static_cast<int>(controller.arms_switch())); }
	update_animation();
	update_sprite();
	walk();
	if (!controller.moving() && (!force_cooldown.running() || get_collider().world_grounded())) { get_collider().physics.acceleration.x = 0.0f; }

	// weapon
	if (controller.is(AbilityType::walljump) && controller.is_ability_active()) { accumulated_forces.push_back({walljump_force_v * controller.get_ability_direction().as_float(), 0.f}); }
	if (controller.shot() || controller.arms_switch()) { m_animation_machine.idle_timer.start(); }
	if (has_flag_set(PlayerFlags::impart_recoil) && arsenal) {
		if (controller.direction.und == UND::down) { accumulated_forces.push_back({0.f, -equipped_weapon().get_recoil()}); }
		if (controller.direction.und == UND::up) { accumulated_forces.push_back({0.f, equipped_weapon().get_recoil()}); }
		set_flag(PlayerFlags::impart_recoil, false);
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
	if (controller.is(AbilityType::dive) && m_services->ticker.every_x_ticks(64) && get_collider().has_flag_set(shape::ColliderFlags::submerged)) {
		map.spawn_emitter(*m_services, "bubble", get_collider().get_center(), Direction{UND::up}, sf::Vector2f{8.f, 8.f});
	}

	update_antennae();

	if (piggybacker) { piggybacker->update(*m_services, *this); }

	if (is_dead()) {
		for (auto& a : antennae) { a->get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, false); }
	}

	// step sounds
	if (m_services->in_game()) {
		if (m_animation_machine.stepped() && abs(get_collider().physics.velocity.x) > 2.5f) { m_services->soundboard.play_step(map.get_tile_value_at_position(get_collider().get_below_point()), map.get_style_id()); }
	}
	Mobile::post_update(*m_services, map, *this);
}

void Player::simple_update() {
	handle_turning();
	tick();
	m_animation_machine.update();
	update_sprite();
	update_antennae();
	m_piggyback_socket = m_sprite_position + sf::Vector2f{-8.f * directions.actual.as_float(), -16.f};
	if (piggybacker) { piggybacker->update(*m_services, *this); }
	update_weapon();
}

void Player::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {

	m_sprite_position = collider.has_value() ? get_collider().get_average_tick_position() + sprite_offset : m_sprite_position;
	m_sprite_position.x += controller.facing_left() ? -1.f : 1.f;
	Animatable::set_position(m_sprite_position - cam);

	if (has_death_type(PlayerDeathType::crushed) || has_death_type(PlayerDeathType::swallowed)) { return; }
	if (has_death_type(PlayerDeathType::drowned)) { set_color(colors::blue); }
	if (piggybacker) { piggybacker->render(svc, win, cam); }

	if (consume_flag(PlayerFlags::dir_switch)) { Animatable::scale({-1.f, 1.f}); }

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
			camera_target.setFillColor(colors::blue);
			camera_target.setPosition(m_ear.physics.position - cam);
			win.draw(camera_target);
			get_collider().render(win, cam);
		}
	} else {
		if (antennae.size() > 1) { antennae[1]->render(svc, win, cam, 1); }
		win.draw(*this);
		if (antennae.size() > 1) { antennae[0]->render(svc, win, cam, 1); }
	}

	if (arsenal && hotbar) {
		if (has_flag_set(PlayerFlags::show_weapon)) { equipped_weapon().render(svc, win, cam); }
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
	if (had_special_death()) { return; }
	health_indicator.render(svc, win, cam);
	orb_indicator.render(svc, win, cam);
	if (get_collider().has_flag_set(shape::ColliderFlags::submerged) || !m_air_supply.full()) { m_air_supply_bar.render(win, cam); }
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
	if (has_flag_set(PlayerFlags::cutscene)) {
		m_animation_machine.update();
		return;
	}
	m_sprite_shake.update();
	set_flag(PlayerFlags::show_weapon);

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
			get_collider().has_flag_set(shape::ColliderFlags::in_water) ? m_animation_machine.request(AnimState::swim) : m_animation_machine.request(AnimState::suspend);
		}
	}

	if (get_collider().physics.apparent_velocity().y > thresholds.suspend && !grounded()) { m_animation_machine.request(AnimState::fall); }

	if (controller.get_ability_animation() && controller.is_ability_active() && controller.is_animation_request()) { m_animation_machine.request(*controller.get_ability_animation()); }

	if (m_animation_machine.is_state(AnimState::sit)) { set_flag(PlayerFlags::show_weapon, false); }
	if (controller.inspecting()) { m_animation_machine.request(AnimState::inspect); }
	if (controller.is_crouching() && grounded()) { controller.moving() ? m_animation_machine.request(AnimState::crawl) : m_animation_machine.request(AnimState::crouch); }
	if (controller.moving() && grounded()) {
		if (get_collider().has_left_wallslide_collision() && controller.horizontal_movement() < 0.f) { cooldowns.push.update(); }
		if (get_collider().has_right_wallslide_collision() && controller.horizontal_movement() > 0.f) { cooldowns.push.update(); }
		if (cooldowns.push.is_complete() && (get_collider().has_right_wallslide_collision() || get_collider().has_left_wallslide_collision())) { m_animation_machine.request(AnimState::push); }
	}

	if (hurt_cooldown.running()) { m_animation_machine.request(AnimState::hurt); }

	if (consume_flag(PlayerFlags::sleep)) { m_animation_machine.request(AnimState::sleep); }
	if (consume_flag(PlayerFlags::wake_up)) { m_animation_machine.request(AnimState::wake_up); }

	if (is_dead()) {
		has_death_type(PlayerDeathType::drowned) ? m_animation_machine.request(AnimState::drown) : m_animation_machine.request(AnimState::die);
		set_flag(PlayerFlags::show_weapon, false);
	}

	m_animation_machine.update();
}

void Player::update_sprite() {

	if (has_collider()) {
		if (!grounded() || controller.is_dashing()) {
			if (directions.desired != directions.actual) { request_flip(); }
		}
	} else {
		if (directions.desired != directions.actual) { request_flip(); }
	}

	Animatable::set_texture(texture_updater.get_dynamic_texture());
}

void Player::handle_turning() {
	if (directions.desired != directions.actual) {
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

void Player::set_slow_walk() {
	m_animation_machine.force(AnimState::slow_walk, "slow_walk");
	m_animation_machine.state_function = std::bind(&PlayerAnimation::update_slow_walk, &m_animation_machine);
}

void Player::set_sleeping() {
	m_animation_machine.force(AnimState::idle, "sleep");
	m_animation_machine.state_function = std::bind(&PlayerAnimation::update_sleep, &m_animation_machine);
	animation.set_frame(3);
}

void Player::set_hurt() {
	m_animation_machine.force(AnimState::hurt, "hurt");
	m_animation_machine.state_function = std::bind(&PlayerAnimation::update_hurt, &m_animation_machine);
}

void Player::set_direction(Direction to) {
	Mobile::set_direction(to);
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
		if (controller.is_wallsliding() && !controller.direction.up_or_down()) { weapon->get_firing_direction().flip(); }
		weapon->update(*m_services, controller.direction);
		weapon->set_position(m_weapon_socket);
	}
}

void Player::walk() {
	if (m_animation_machine.is_state(AnimState::slide)) { return; }
	if (m_animation_machine.is_state(AnimState::dash)) { return; }
	if (m_animation_machine.is_state(AnimState::roll)) { return; }
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
		m_services->ticker.freeze_frame(12 * std::min(static_cast<int>(amount), 3));
		m_sprite_shake.start();
		m_hurt_cooldown.start();
		health.inflict(amount, force);
		health_indicator.add(-amount);
		get_collider().physics.velocity.y = 0.0f;
		get_collider().physics.acceleration.y += -physics_stats.hurt_acc;
		force_cooldown.start(60);
		has_death_type(PlayerDeathType::swallowed) || has_death_type(PlayerDeathType::drowned) ? m_services->soundboard.flags.player.set(audio::Player::gulp) : m_services->soundboard.flags.player.set(audio::Player::hurt);
		hurt_cooldown.start(2);
		if (health.is_dead()) { m_death_type = PlayerDeathType::normal; }
	}
}

void Player::on_crush(world::Map& map) {
	if (!get_collider().collision_depths) { return; }
	if (get_collider().has_flag_set(shape::ColliderFlags::crushed) && alive()) {
		hurt(max_damage_v, true);
		left_squish.und = get_collider().horizontal_squish() ? UND::up : UND::neutral;
		left_squish.lnr = get_collider().vertical_squish() ? LNR::left : LNR::neutral;
		right_squish.und = get_collider().horizontal_squish() ? UND::down : UND::neutral;
		right_squish.lnr = get_collider().vertical_squish() ? LNR::right : LNR::neutral;
		map.spawn_emitter(*m_services, "player_crush", get_collider().physics.position, left_squish, get_collider().dimensions);
		map.spawn_emitter(*m_services, "player_crush", get_collider().physics.position, right_squish, get_collider().dimensions);
		get_collider().collision_depths = {};
		m_death_type = PlayerDeathType::crushed;
	}
}

void Player::handle_map_collision(world::Map& map) { get_collider().detect_map_collision(map); }

void Player::update_antennae() {
	auto position = m_sprite_position;
	for (auto [i, a] : std::views::enumerate(antennae)) {
		a->attraction_force = physics_stats.antenna_force;
		a->get_collider().physics.set_friction_componentwise({physics_stats.antenna_friction, physics_stats.antenna_friction});
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
		auto sign = directions.desired.as_float();
		socket.x = i == 0 ? 10.0f * sign : -3.f * sign;

		if (m_animation_machine.get_frame() == 82) { socket.x += controller.facing_right() ? 6.f : -6.f; }
		if (!is_dead()) {
			a->set_target_position(position + socket);
		} else {
			a->demagnetize(*m_services);
		}
		a->update(*m_services);
	}
}

void Player::sync_antennae() {
	auto position = m_sprite_position;
	for (auto [i, a] : std::views::enumerate(antennae)) {
		auto& socket = i == 0 ? m_antenna_sockets.first : m_antenna_sockets.second;
		a->set_position(position + socket);
		a->update(*m_services);
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
		set_flag(PlayerFlags::impart_recoil);
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
	m_death_type.reset();
	m_air_supply.refill();
	health.reset();
	controller.unrestrict();
	m_services->camera_controller.set_owner(graphics::CameraOwner::player);
	health.invincibility.start(8);
	hurt_cooldown.cancel();
	set_flag(PlayerFlags::killed, false);
	set_flag(PlayerFlags::cutscene, false);
	m_animation_machine.triggers.reset(AnimTriggers::end_death);
	set_animation_flag(player::AnimTriggers::end_death, false);
	m_animation_machine.post_death.cancel();
	if (has_collider()) { get_collider().collision_depths = util::CollisionDepth(); }
	for (auto& a : antennae) {
		a->get_collider().physics.set_global_friction(physics_stats.antenna_friction);
		a->get_collider().physics.gravity = 0.f;
	}
	sync_antennae();
	catalog.wardrobe.set_palette(m_services->assets.get_texture_modifiable("nani_palette_default"));
	update_wardrobe();
	set_idle();
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

void Player::give_item_by_id(int id, int amount) { give_item(m_services->data.item_label_from_id(id), amount); }

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
	for (auto i{0}; i < amount; ++i) { catalog.inventory.add_item(m_services->data.item, label); }
	if (label == "heart_keychain" && !from_save) {
		health.increase_capacity(1.f);
		m_services->soundboard.flags.item.set(audio::Item::health_increase);
		health.refill();
	}
}

EquipmentStatus Player::equip_item(int id) { return catalog.inventory.equip_item(id); }

void Player::reset_flags() {
	flags = {};
	reset_all();
}

void Player::reset_water_flags() {
	if (!has_collider()) { return; }
	get_collider().set_flag(shape::ColliderFlags::in_water, false);
	get_collider().set_flag(shape::ColliderFlags::submerged, false);
}

void Player::total_reset() {
	start_over();
	get_collider().physics.zero();
	reset_flags();
	arsenal = {};
	update_antennae();
	set_flag(PlayerFlags::killed, false);
	set_flag(PlayerFlags::cutscene, false);
	set_idle();
}

void Player::map_reset() {
	if (!m_services->app_flags.test(automa::AppFlags::custom_map_start)) { set_idle(); }
	set_flag(PlayerFlags::killed, false);
	set_flag(PlayerFlags::cutscene, false);
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
	if (!has_flag_set(PlayerFlags::dash_kick)) { return false; }
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
	if (grounded() && !controller.is(AbilityType::dash)) { return false; }
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
	if (get_collider().grounded()) { return false; }
	if (get_collider().physics.apparent_velocity().y < wallslide_threshold_v) { return false; }
	if (!catalog.inventory.has_item("kariba_talisman")) { return false; }
	return true;
}

bool Player::can_walljump() const {
	if (health.is_dead()) { return false; }
	if (!catalog.inventory.has_item("kariba_talisman")) { return false; }
	return true;
}

bool Player::can_dive() const {
	if (health.is_dead()) { return false; }
	if (!get_collider().has_flag_set(shape::ColliderFlags::in_water)) { return false; }
	return true;
}

} // namespace fornani::player
