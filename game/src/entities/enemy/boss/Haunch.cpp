
#include <fornani/entities/enemy/boss/Haunch.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto haunch_framerate{14};

Haunch::Haunch(automa::ServiceProvider& svc, world::Map& map)
	: Boss{svc, map, "haunch"}, Animatable{svc, "enemy_haunch", {79, 80}}, m_services{&svc}, m_map{&map}, m_gun{svc, "big_laser_gun"}, m_stun_grenade{svc, "stun_grenade"}, m_hand_grenade{svc, "hand_grenade"},
	  m_cooldowns{.run{200}, .post_run{1800}, .grenade{40}, .laser_charge{288}, .post_laser{96}, .whistle{400}, .post_whistle{1400}, .post_death{1000}}, m_laser_gun{svc, "haunch_laser_gun", {37, 15}} {
	p_animations = {
		{"idle", {0, 6, haunch_framerate * 3, -1}},
		{"turn", {18, 1, haunch_framerate * 3, 0}},
		{"shoot_high", {6, 3, haunch_framerate * 4, 0, true}},
		{"shoot_low", {10, 4, haunch_framerate * 3, 0, true}},
		{"get_up", {14, 5, haunch_framerate * 2, 0}},
		{"airborne", {22, 1, haunch_framerate, -1}},
		{"walk", {19, 4, haunch_framerate * 2, -1}},
		{"pull_grenade", {23, 3, haunch_framerate * 3, 0}},
		{"throw_grenade", {26, 3, haunch_framerate * 2, 0}},
		{"triple_toss", {26, 3, haunch_framerate, 2}},
		{"throw_grenade_down", {29, 5, haunch_framerate * 2, 0}},
		{"triple_down_toss", {29, 5, haunch_framerate, 2}},
		{"whistle", {34, 8, haunch_framerate * 2, 0}},
		{"struggle", {42, 1, haunch_framerate * 2, 0}},
		{"stalk", {19, 4, haunch_framerate * 2, -1}},
	};
	animation.set_params(get_params("airborne"));
	get_collider().physics.set_friction_componentwise({0.99f, 1.f});
	set_direction({LR::right});
	m_gun.get().set_team(arms::Team::skycorps);
	m_stun_grenade.get().set_team(arms::Team::skycorps);
	m_hand_grenade.get().set_team(arms::Team::skycorps);
	m_laser_gun.push_animation("fire", {10, 5, 32, 0});
	m_laser_gun.push_animation("charge", {1, 9, 32, 0});
	m_laser_gun.push_and_set_animation("neutral", {0, 1, 32, -1});
	m_laser_gun.center();
	flags.state.set(StateFlags::no_shake);
	flags.general.set(GeneralFlags::post_death_render);
}

void Haunch::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Boss::update(svc, map, player);
	if (consume_flag(BossFlags::start_battle)) { svc.data.switch_destructible_state(90103, true); }
	if (has_flag_set(BossFlags::end_battle) && !has_flag_set(BossFlags::post_death)) {
		svc.music_player.stop();
		set_flag(BossFlags::post_death);
		svc.music_player.load(svc.finder, "wind");
		svc.music_player.play_looped();
		request(HaunchState::struggle);
		map.clear_projectiles();
		map.clear_enemies({28});
	}
	if (has_flag_set(BossFlags::post_death)) {
		flags.state.set(StateFlags::special_death_mode);
		m_flags.reset(HaunchFlags::show_gun);
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		if (m_dynamite_stick) { m_dynamite_stick->tick(); }
		shake();
		m_cooldowns.post_death.update();
		if (!m_cooldowns.post_death.running()) { m_cooldowns.post_death.start(); }
		if (svc.ticker.every_x_ticks(70)) {
			auto pos = get_collider().get_center() + random::random_vector_float(-40.f, 40.f);
			map.spawn_effect(svc, "puff", pos, {}, 1);
		}
		if (flags.state.consume(StateFlags::special_event)) {
			request(HaunchState::stalk);
			if (!m_dynamite_stick) {
				m_dynamite_stick.emplace(svc, "tnt", sf::Vector2i{18, 22});
				m_dynamite_stick->push_and_set_animation("charged", {0, 6, 22, -1});
				m_dynamite_stick->center();
			}
		}
	}
	if (m_cooldowns.post_death.is_almost_complete()) {
		if (!m_flags.test(HaunchFlags::escape_cutscene_launched) && !player.is_dead()) {
			svc.events.launch_cutscene_event.dispatch(svc, 902);
			m_flags.set(HaunchFlags::escape_cutscene_launched);
		}
		set_flag(BossFlags::post_death, false);
	}
	auto bp = get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * -8.f, -16.f};
	auto offset = is_state(HaunchState::shoot_low)	  ? sf::Vector2f{directions.actual.as_float() * 38.f, 42.f}
				  : is_state(HaunchState::shoot_high) ? sf::Vector2f{directions.actual.as_float() * 48.f, -6.f}
													  : sf::Vector2f{directions.actual.as_float() * 18.f, 0.f};
	m_gun_socket = bp + offset;
	if (!has_flag_set(BossFlags::battle_mode)) {
		m_gun_steering.seek(m_gun_socket);
		state_function = state_function();
		return;
	}

	m_cooldowns.grenade.update();
	m_cooldowns.run.update();
	m_cooldowns.post_run.update();
	m_cooldowns.laser_charge.update();
	m_cooldowns.post_laser.update();
	m_cooldowns.whistle.update();
	m_cooldowns.post_whistle.update();

	m_laser_gun.tick();
	if (m_gun.get().has_flag_set(arms::WeaponFlags::charging)) {
		auto progress = static_cast<int>(std::round(m_cooldowns.laser_charge.get_inverse_normalized() * 9.f)) - 1;
		m_laser_gun.set_frame(progress);
	}
	if (m_laser_gun.animation.is_complete() && m_flags.test(HaunchFlags::laser_fired)) {
		m_laser_gun.set_animation("neutral");
		m_flags.reset(HaunchFlags::laser_fired);
	}

	m_flags.reset(HaunchFlags::show_gun);
	if (is_state(HaunchState::shoot_low) || is_state(HaunchState::shoot_high) || is_state(HaunchState::walk) || is_state(HaunchState::idle) || is_state(HaunchState::airborne) || is_state(HaunchState::get_up)) {
		m_flags.set(HaunchFlags::show_gun);
	}

	m_stun_grenade.update(svc, map, *this);
	m_hand_grenade.update(svc, map, *this);
	m_stun_grenade.get().set_firing_direction(CardinalDirection{UDLR::down});
	m_hand_grenade.get().set_firing_direction(CardinalDirection{UDLR::up});
	m_gun.update(svc, map, *this);
	m_stun_grenade.get().set_barrel_point(get_collider().get_center());
	m_hand_grenade.get().set_barrel_point(get_collider().get_center());
	m_attack_target = player.get_collider().get_center() + sf::Vector2f{0.f, -20.f} - bp;
	m_gun.get().set_barrel_point(m_gun_socket + sf::Vector2f{36.f * directions.actual.as_float(), -8.f});
	m_laser_gun.set_scale({-get_scale().x, get_scale().y});
	m_gun_steering.seek(m_gun_socket);

	if (is_alert()) {
		if (half_health()) {
			random::percent_chance(60) && !m_cooldowns.post_whistle.running() ? request(HaunchState::whistle) : request(HaunchState::pull_grenade);
		} else {
			if (random::percent_chance(40) && !m_cooldowns.run.running() && !m_cooldowns.post_run.running()) {
				m_cooldowns.run.start();
			} else {
				request(HaunchState::pull_grenade);
			}
		}
	}
	if (is_hostile()) { half_health() ? request(HaunchState::triple_down_toss) : request(HaunchState::throw_grenade_down); }
	if (!is_alert() && !is_hostile()) {
		if (half_health() && !m_cooldowns.post_whistle.running()) {
			request(HaunchState::whistle);
		} else {
			random::percent_chance(70) ? m_cooldowns.run.start(80) : request(HaunchState::pull_grenade);
		}
	}
	if (m_cooldowns.run.running()) { request(HaunchState::walk); }
	if (player.get_collider().get_center().y < get_collider().get_top().y && player.controller.is_wallsliding()) { request(HaunchState::triple_down_toss); }

	if (secondary_collider) {
		get_secondary_collider().physics.position = get_collider().get_top() - get_secondary_collider().dimensions * 0.5f;
		get_secondary_collider().sync_components();
	}

	face_player(player);
	flags.state.set(StateFlags::vulnerable);

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(HaunchState::turn); }
	if (has_flag_set(BossFlags::post_death) && !m_flags.test(HaunchFlags::escape_cutscene_launched)) { request(HaunchState::struggle); }

	state_function = state_function();
}

void Haunch::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	m_laser_gun.set_position(m_gun_steering.physics.position - cam);
	if (m_flags.test(HaunchFlags::show_gun)) { win.draw(m_laser_gun); }
	if (svc.greyblock_mode()) {}
	if (m_dynamite_stick) {
		m_dynamite_stick->set_position(get_collider().get_center() - cam);
		win.draw(*m_dynamite_stick);
	}
}

void Haunch::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	// debug();
}

void Haunch::debug() {
	static auto sz = ImVec2{180.f, 450.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Haunch Debug")) {
		if (ImGui::Button("Start Battle")) { start_battle(); }
		ImGui::SeparatorText("Info");
		ImGui::Text("Run: %i", m_cooldowns.run.get());
		ImGui::Text("Post Run: %i", m_cooldowns.post_run.get());
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("get up")) { request(HaunchState::get_up); }
		if (ImGui::Button("shoot high")) { request(HaunchState::shoot_high); }
		if (ImGui::Button("shoot low")) { request(HaunchState::shoot_low); }
		if (ImGui::Button("pull grenade")) { request(HaunchState::pull_grenade); }
		if (ImGui::Button("throw grenade")) { request(HaunchState::throw_grenade); }
		if (ImGui::Button("throw grenade down")) { request(HaunchState::throw_grenade_down); }
		if (ImGui::Button("walk")) { request(HaunchState::walk); }
		ImGui::End();
	}
}

fsm::StateFunction Haunch::update_idle() {
	p_state.actual = HaunchState::idle;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (change_state(HaunchState::whistle, get_params("whistle"))) { return HAUNCH_BIND(update_whistle); }
	if (change_state(HaunchState::walk, get_params("walk"))) { return HAUNCH_BIND(update_walk); }
	if (change_state(HaunchState::turn, get_params("turn"))) { return HAUNCH_BIND(update_turn); }
	if (change_state(HaunchState::pull_grenade, get_params("pull_grenade"))) { return HAUNCH_BIND(update_pull_grenade); }
	if (change_state(HaunchState::triple_down_toss, get_params("triple_down_toss")) && get_collider().grounded()) { return HAUNCH_BIND(update_triple_down_toss); }
	if (change_state(HaunchState::throw_grenade_down, get_params("throw_grenade_down")) && get_collider().grounded()) { return HAUNCH_BIND(update_throw_grenade_down); }
	return HAUNCH_BIND(update_idle);
}

fsm::StateFunction Haunch::update_airborne() {
	p_state.actual = HaunchState::airborne;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (change_state(HaunchState::triple_down_toss, get_params("triple_down_toss")) && get_collider().grounded()) { return HAUNCH_BIND(update_triple_down_toss); }
	if (change_state(HaunchState::throw_grenade_down, get_params("throw_grenade_down")) && get_collider().grounded()) { return HAUNCH_BIND(update_throw_grenade_down); }
	if (get_collider().grounded()) {
		request(HaunchState::idle);
		if (change_state(HaunchState::idle, get_params("idle"))) { return HAUNCH_BIND(update_idle); }
	}
	return HAUNCH_BIND(update_airborne);
}

fsm::StateFunction Haunch::update_turn() {
	p_state.actual = HaunchState::turn;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (animation.complete()) {
		request_flip();
		if (m_flags.test(HaunchFlags::escape_cutscene_launched)) {
			request(HaunchState::stalk);
			if (change_state(HaunchState::stalk, get_params("stalk"))) { return HAUNCH_BIND(update_stalk); }
		}
		if (change_state(HaunchState::triple_down_toss, get_params("triple_down_toss")) && get_collider().grounded()) { return HAUNCH_BIND(update_triple_down_toss); }
		if (change_state(HaunchState::throw_grenade_down, get_params("throw_grenade_down")) && get_collider().grounded()) { return HAUNCH_BIND(update_throw_grenade_down); }
		request(HaunchState::idle);
		if (change_state(HaunchState::idle, get_params("idle"))) { return HAUNCH_BIND(update_idle); }
	}
	return HAUNCH_BIND(update_turn);
}

fsm::StateFunction Haunch::update_shoot_high() {
	p_state.actual = HaunchState::shoot_high;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	shoot_gun();
	if (change_state(HaunchState::idle, get_params("idle"))) { return HAUNCH_BIND(update_idle); }
	return HAUNCH_BIND(update_shoot_high);
}

fsm::StateFunction Haunch::update_shoot_low() {
	p_state.actual = HaunchState::shoot_low;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (animation.get_frame_count() == 1 && animation.keyframe_started()) { m_services->soundboard.play_sound("thud", get_collider().get_center()); }
	shoot_gun();
	if (change_state(HaunchState::get_up, get_params("get_up"))) { return HAUNCH_BIND(update_get_up); }
	return HAUNCH_BIND(update_shoot_low);
}

fsm::StateFunction Haunch::update_get_up() {
	p_state.actual = HaunchState::get_up;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (animation.is_complete()) {
		if (change_state(HaunchState::walk, get_params("walk"))) { return HAUNCH_BIND(update_walk); }
		request(HaunchState::idle);
		if (change_state(HaunchState::idle, get_params("idle"))) { return HAUNCH_BIND(update_idle); }
	}
	return HAUNCH_BIND(update_get_up);
}

fsm::StateFunction Haunch::update_walk() {
	p_state.actual = HaunchState::walk;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (!get_collider().grounded()) { request(HaunchState::airborne); }
	if (change_state(HaunchState::airborne, get_params("airborne"))) { return HAUNCH_BIND(update_airborne); }
	if (animation.get_frame_count() % 2 == 0 && animation.keyframe_started()) { m_services->soundboard.play_sound("tank_step", get_collider().get_center()); }
	get_collider().physics.apply_force({directions.actual.as_float(), 0.f});
	if (change_state(HaunchState::turn, get_params("turn"))) { return HAUNCH_BIND(update_turn); }
	if (m_cooldowns.run.is_almost_complete()) { m_cooldowns.post_run.start(); }
	if (m_cooldowns.run.running()) { return HAUNCH_BIND(update_walk); }
	if (change_state(HaunchState::pull_grenade, get_params("pull_grenade"))) { return HAUNCH_BIND(update_pull_grenade); }
	return HAUNCH_BIND(update_walk);
}

fsm::StateFunction Haunch::update_pull_grenade() {
	p_state.actual = HaunchState::pull_grenade;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (animation.just_started()) { m_services->soundboard.play_sound("grenade_pin"); }
	if (animation.is_complete()) {
		half_health() ? (random::percent_chance(65) ? request(HaunchState::triple_toss) : request(HaunchState::throw_grenade)) : request(HaunchState::throw_grenade);
		if (change_state(HaunchState::triple_toss, get_params("triple_toss"))) { return HAUNCH_BIND(update_triple_toss); }
		if (change_state(HaunchState::throw_grenade, get_params("throw_grenade"))) { return HAUNCH_BIND(update_throw_grenade); }
	}
	return HAUNCH_BIND(update_pull_grenade);
}

fsm::StateFunction Haunch::update_throw_grenade() {
	p_state.actual = HaunchState::throw_grenade;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (animation.just_started() && !m_cooldowns.grenade.running()) {
		m_hand_grenade.shoot(*m_services, *m_map, m_attack_target);
		m_services->soundboard.play_sound("grenade_toss", get_collider().get_center());
		m_cooldowns.grenade.start();
		get_collider().physics.apply_force({directions.actual.as_float() * 10.f, -12.f});
	}
	if (animation.complete()) {
		if (change_state(HaunchState::triple_down_toss, get_params("triple_down_toss")) && get_collider().grounded()) { return HAUNCH_BIND(update_triple_down_toss); }
		if (change_state(HaunchState::throw_grenade_down, get_params("throw_grenade_down")) && get_collider().grounded()) { return HAUNCH_BIND(update_throw_grenade_down); }
		random::coin_flip() ? request(HaunchState::shoot_low) : request(HaunchState::shoot_high);
		if (change_state(HaunchState::shoot_high, get_params("shoot_high"))) { return HAUNCH_BIND(update_shoot_high); }
		if (change_state(HaunchState::shoot_low, get_params("shoot_low"))) { return HAUNCH_BIND(update_shoot_low); }
	}
	return HAUNCH_BIND(update_throw_grenade);
}

fsm::StateFunction Haunch::update_triple_toss() {
	p_state.actual = HaunchState::triple_toss;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (animation.just_started()) { get_collider().physics.apply_force({directions.actual.as_float() * -10.f, -60.f}); }
	if (animation.get_frame_count() == 0 && !m_cooldowns.grenade.running()) {
		m_hand_grenade.shoot(*m_services, *m_map, m_attack_target);
		m_services->soundboard.play_sound("grenade_toss", get_collider().get_center());
		m_cooldowns.grenade.start();
	}
	if (animation.complete()) {
		get_collider().grounded() ? (random::coin_flip() ? request(HaunchState::shoot_low) : request(HaunchState::shoot_high)) : request(HaunchState::airborne);
		if (change_state(HaunchState::shoot_high, get_params("shoot_high"))) { return HAUNCH_BIND(update_shoot_high); }
		if (change_state(HaunchState::shoot_low, get_params("shoot_low"))) { return HAUNCH_BIND(update_shoot_low); }
		if (change_state(HaunchState::airborne, get_params("airborne"))) { return HAUNCH_BIND(update_airborne); }
	}
	return HAUNCH_BIND(update_triple_toss);
}

fsm::StateFunction Haunch::update_throw_grenade_down() {
	p_state.actual = HaunchState::throw_grenade_down;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (animation.get_frame_count() == 2 && !m_cooldowns.grenade.running()) {
		m_stun_grenade.shoot(*m_services, *m_map, m_attack_target);
		m_services->soundboard.play_sound("grenade_toss", get_collider().get_center());
		m_cooldowns.grenade.start();
	}
	if (animation.just_started() && !m_flags.test(HaunchFlags::jumped)) {
		get_collider().physics.apply_force({directions.actual.as_float() * 20.f, -100.f});
		m_flags.set(HaunchFlags::jumped);
	}
	if (animation.is_complete()) {
		m_flags.reset(HaunchFlags::jumped);
		get_collider().grounded() ? request(HaunchState::idle) : request(HaunchState::airborne);
		if (change_state(HaunchState::airborne, get_params("airborne"))) { return HAUNCH_BIND(update_airborne); }
		if (change_state(HaunchState::idle, get_params("idle"))) { return HAUNCH_BIND(update_idle); }
	}
	return HAUNCH_BIND(update_throw_grenade_down);
}

fsm::StateFunction Haunch::update_whistle() {
	p_state.actual = HaunchState::whistle;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (animation.get_frame_count() == 1 && animation.keyframe_started()) { m_services->soundboard.play_sound("haunch_whistle", get_collider().get_center()); }
	if (animation.get_frame_count() == 5 && !m_cooldowns.whistle.running()) {
		for (auto i = 0; i < 3; ++i) {
			auto pos = get_collider().get_top() + sf::Vector2f{0.f, -140.f} + random::random_vector_float(-200.f, 200.f);
			m_map->spawn_enemy(19, pos);
		}
		m_cooldowns.whistle.start();
	}
	if (animation.is_complete()) {
		m_cooldowns.post_whistle.start();
		request(HaunchState::idle);
		if (change_state(HaunchState::idle, get_params("idle"))) { return HAUNCH_BIND(update_idle); }
	}
	return HAUNCH_BIND(update_whistle);
}

fsm::StateFunction Haunch::update_triple_down_toss() {
	p_state.actual = HaunchState::triple_down_toss;
	if (change_state(HaunchState::struggle, get_params("struggle"))) { return HAUNCH_BIND(update_struggle); }
	if (animation.just_started()) { get_collider().physics.apply_force({directions.actual.as_float() * -60.f, -150.f}); }
	if (animation.get_frame_count() == 2 && !m_cooldowns.grenade.running()) {
		m_stun_grenade.shoot(*m_services, *m_map, m_attack_target);
		m_services->soundboard.play_sound("grenade_toss", get_collider().get_center());
		m_cooldowns.grenade.start();
	}
	if (animation.complete()) {
		if (change_state(HaunchState::walk, get_params("walk"))) { return HAUNCH_BIND(update_walk); }
		get_collider().grounded() ? (random::coin_flip() ? request(HaunchState::shoot_low) : request(HaunchState::shoot_high)) : request(HaunchState::airborne);
		if (change_state(HaunchState::shoot_high, get_params("shoot_high"))) { return HAUNCH_BIND(update_shoot_high); }
		if (change_state(HaunchState::shoot_low, get_params("shoot_low"))) { return HAUNCH_BIND(update_shoot_low); }
		if (change_state(HaunchState::airborne, get_params("airborne"))) { return HAUNCH_BIND(update_airborne); }
	}
	return HAUNCH_BIND(update_triple_down_toss);
}

fsm::StateFunction Haunch::update_struggle() {
	p_state.actual = HaunchState::struggle;
	if (change_state(HaunchState::stalk, get_params("stalk"))) { return HAUNCH_BIND(update_stalk); }
	return HAUNCH_BIND(update_struggle);
}

fsm::StateFunction Haunch::update_stalk() {
	p_state.actual = HaunchState::stalk;
	if (animation.get_frame_count() % 2 == 0 && animation.keyframe_started()) { m_services->soundboard.play_sound("tank_step", get_collider().get_center()); }
	get_collider().physics.apply_force({directions.actual.as_float(), 0.f});
	if (directions.actual.lnr != directions.desired.lnr) { request(HaunchState::turn); }
	if (change_state(HaunchState::turn, get_params("turn"))) { return HAUNCH_BIND(update_turn); }
	return HAUNCH_BIND(update_stalk);
}

void Haunch::shoot_gun() {
	if (animation.just_started()) {
		m_cooldowns.laser_charge.start();
		m_laser_gun.set_animation("charge");
	}
	if (m_cooldowns.laser_charge.running()) { m_services->soundboard.repeat_sound("charge_laser_gun", 1, m_gun.get().get_barrel_point()); }
	if (m_cooldowns.laser_charge.is_almost_complete()) {
		m_laser_gun.set_animation("fire");
		m_flags.set(HaunchFlags::laser_fired);
		auto attributes = util::BitFlags<world::LaserAttributes>{};
		attributes.set(world::LaserAttributes::no_collision);
		m_map->spawn_laser(*m_services, m_gun.get().get_barrel_point(), m_gun.get().get_firing_direction(), arms::LaserSpecifications{{}, 80, 128, 0.75, 1.f, attributes});
		m_cooldowns.post_laser.start();
		m_services->soundboard.play_sound("release_laser_gun");
	}
	if (m_cooldowns.post_laser.is_almost_complete()) { is_state(HaunchState::shoot_low) ? request(HaunchState::get_up) : request(HaunchState::idle); }
}

bool Haunch::change_state(HaunchState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
