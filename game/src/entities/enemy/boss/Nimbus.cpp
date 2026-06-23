
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/enemy/boss/Nimbus.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Nimbus::Nimbus(automa::ServiceProvider& svc, world::Map& map) : Boss(svc, map, "nimbus"), m_slash_wave(svc, "slash_wave"), m_map{&map}, m_services{&svc} {
	auto fr = 9;
	p_animations = {{"idle", {93, 4, fr * 4, -1}},
					{"jump", {1, 7, fr * 5, 0, true}},
					{"land", {8, 7, fr * 3, 0}},
					{"get_up", {12, 2, fr * 4, 0}},
					{"prepare_forward_slash", {16, 3, fr * 5, 0}},
					{"forward_slash", {19, 9, fr * 2, 0}},
					{"prepare_twirl_cut", {29, 4, fr * 5, 0}},
					{"twirl_cut", {33, 6, fr * 2, 0}},
					{"flourish", {39, 9, fr * 3, 0}},
					{"downward_cut", {48, 8, fr * 3, 0}},
					{"upward_cut", {56, 6, fr * 3, 0}},
					{"turn", {62, 7, fr * 2, 0}},
					{"parry", {69, 3, fr * 4, 0}},
					{"begin_levitate", {72, 5, fr * 2, 0}},
					{"levitate", {77, 4, fr * 4, -1}},
					{"spellcast", {81, 3, fr * 2, 3}},
					{"double_aerial_slash", {84, 9, fr * 2, 0}}};
	animation.set_params(get_params("idle"));
	get_collider().physics.set_friction_componentwise({0.99f, 0.999f});
	m_slash_wave.get().set_team(arms::Team::skycorps);
	flags.general.set(GeneralFlags::has_invincible_channel);
	flags.state.set(StateFlags::vulnerable);
	start_battle();
}

void Nimbus::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Boss::update(svc, map, player);

	// logic
	m_cooldowns.post_slash.update();
	if (is_airborne()) { m_steering.thrust_seek(get_collider().physics, m_steer_target, {0.03f, .118f, .991f, 260.f}); }
	m_slash_wave.update(svc, map, *this);
	auto bp = Enemy::get_collider().get_center();
	m_slash_wave.get().set_barrel_point(bp);

	// melee attacks
	if (Boss::has_flag_set(BossFlags::battle_mode)) {
		for (auto& slash : m_attacks.slash) {
			auto damage = 1.f;
			slash.disable();
			slash.update();
			if (is_state(NimbusState::upward_cut)) {
				slash.enable();
				if (animation.get_frame_count() != 1) { slash.disable(); }
				damage = 1.f;
			}
			if (is_state(NimbusState::turn)) {
				slash.enable();
				if (animation.get_frame_count() != 3 && animation.get_frame_count() != 4) { slash.disable(); }
				damage = 2.f;
			}
			if (is_state(NimbusState::forward_slash)) {
				slash.enable();
				if (animation.get_frame_count() != 1 && animation.get_frame_count() != 2) { slash.disable(); }
				damage = 2.f;
			}
			if (is_state(NimbusState::downward_cut)) {
				slash.enable();
				if (animation.get_frame_count() != 3 && animation.get_frame_count() != 4) { slash.disable(); }
				damage = 2.f;
			}
			slash.hurt_player(player, damage, {directions.desired.as_float() * 0.4f, -0.2f});
			slash.cancel_projectiles(svc, map, get_team(), 4);
		}
	}

	if (svc.ticker.every_second()) {
		if (!m_cooldowns.post_slash.running()) {
			random::coin_flip() ? request(NimbusState::prepare_forward_slash) : request(NimbusState::prepare_twirl_cut);
			if (random::percent_chance(25)) { request(NimbusState::flourish); }
		}
	}

	// animation
	if (m_caution.is_projectile_detected(map, physical.alert_range, arms::Team::skycorps)) {
		request(NimbusState::parry);
	} else if (is_state(NimbusState::parry)) {
		request(NimbusState::idle);
	}
	if (is_hostile()) { request(NimbusState::upward_cut); }
	if (directions.actual.lnr != directions.desired.lnr && !is_airborne()) { request(NimbusState::turn); }

	state_function = state_function();
}

void Nimbus::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::render(svc, win, cam);
	if (svc.greyblock_mode()) {
		for (auto& slash : m_attacks.slash) {
			if (slash.hit.active()) { slash.render(win, cam); }
		}
	}
}

void Nimbus::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	debug();
}

fsm::StateFunction Nimbus::update_idle() {
	p_state.actual = NimbusState::idle;
	if (change_state(NimbusState::jump, Enemy::get_params("jump"))) { return NIMBUS_BIND(update_jump); }
	if (change_state(NimbusState::prepare_forward_slash, Enemy::get_params("prepare_forward_slash"))) { return NIMBUS_BIND(update_prepare_forward_slash); }
	if (change_state(NimbusState::prepare_twirl_cut, Enemy::get_params("prepare_twirl_cut"))) { return NIMBUS_BIND(update_prepare_twirl_cut); }
	if (change_state(NimbusState::flourish, Enemy::get_params("flourish"))) { return NIMBUS_BIND(update_flourish); }
	if (change_state(NimbusState::downward_cut, Enemy::get_params("downward_cut"))) { return NIMBUS_BIND(update_downward_cut); }
	if (change_state(NimbusState::upward_cut, Enemy::get_params("upward_cut"))) { return NIMBUS_BIND(update_upward_cut); }
	if (change_state(NimbusState::turn, Enemy::get_params("turn"))) { return NIMBUS_BIND(update_turn); }
	if (change_state(NimbusState::parry, Enemy::get_params("parry"))) { return NIMBUS_BIND(update_parry); }
	if (change_state(NimbusState::begin_levitate, Enemy::get_params("begin_levitate"))) { return NIMBUS_BIND(update_begin_levitate); }
	return NIMBUS_BIND(update_idle);
}

fsm::StateFunction Nimbus::update_jump() {
	p_state.actual = NimbusState::jump;
	if (animation.get_frame_count() == 1 && animation.keyframe_started()) { get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * 5.f, -15.f}; }
	if (animation.get_frame_count() >= 1) { get_collider().physics.apply_force({directions.actual.as_float(), 0.f}); }
	if (animation.get_frame_count() == 6 && is_alert()) { request(NimbusState::downward_cut); }
	if (change_state(NimbusState::downward_cut, get_params("downward_cut"))) { return NIMBUS_BIND(update_downward_cut); }
	if (animation.is_complete() && get_collider().grounded()) {
		request(NimbusState::land);
		if (change_state(NimbusState::land, Enemy::get_params("land"))) { return NIMBUS_BIND(update_land); }
	}
	return NIMBUS_BIND(update_jump);
}

fsm::StateFunction Nimbus::update_land() {
	p_state.actual = NimbusState::land;
	if (animation.is_complete()) {
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_land);
}

fsm::StateFunction Nimbus::update_get_up() {
	p_state.actual = NimbusState::get_up;
	if (animation.is_complete()) {
		if (change_state(NimbusState::turn, Enemy::get_params("turn"))) { return NIMBUS_BIND(update_turn); }
		if (!m_cooldowns.post_slash.running()) { random::coin_flip() ? request(NimbusState::prepare_forward_slash) : request(NimbusState::prepare_twirl_cut); }
		if (change_state(NimbusState::prepare_forward_slash, Enemy::get_params("prepare_forward_slash"))) { return NIMBUS_BIND(update_prepare_forward_slash); }
		if (change_state(NimbusState::prepare_twirl_cut, Enemy::get_params("prepare_twirl_cut"))) { return NIMBUS_BIND(update_prepare_twirl_cut); }
		if (random::percent_chance(25)) { request(NimbusState::jump); }
		if (change_state(NimbusState::jump, Enemy::get_params("jump"))) { return NIMBUS_BIND(update_jump); }
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_get_up);
}

fsm::StateFunction Nimbus::update_prepare_forward_slash() {
	p_state.actual = NimbusState::prepare_forward_slash;
	if (animation.is_complete()) {
		request(NimbusState::forward_slash);
		if (change_state(NimbusState::forward_slash, Enemy::get_params("forward_slash"))) { return NIMBUS_BIND(update_forward_slash); }
	}
	return NIMBUS_BIND(update_prepare_forward_slash);
}

fsm::StateFunction Nimbus::update_forward_slash() {
	p_state.actual = NimbusState::forward_slash;
	if (animation.just_started()) { get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * 40.f, -7.f}; }
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 28.f, 8.f});
		i == 1 ? slash.set_constant_radius(40.f) : slash.set_constant_radius(18.f);
	}
	if (animation.is_complete()) {
		m_cooldowns.post_slash.start();
		if (change_state(NimbusState::turn, Enemy::get_params("turn"))) { return NIMBUS_BIND(update_turn); }
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_forward_slash);
}

fsm::StateFunction Nimbus::update_prepare_twirl_cut() {
	p_state.actual = NimbusState::prepare_twirl_cut;
	if (animation.is_complete()) {
		request(NimbusState::twirl_cut);
		if (change_state(NimbusState::twirl_cut, Enemy::get_params("twirl_cut"))) { return NIMBUS_BIND(update_twirl_cut); }
	}
	return NIMBUS_BIND(update_prepare_twirl_cut);
}

fsm::StateFunction Nimbus::update_twirl_cut() {
	p_state.actual = NimbusState::twirl_cut;
	if (animation.just_started()) { m_slash_wave.shoot(*m_services, *m_map, (get_collider().get_center() + sf::Vector2f{directions.actual.as_float(), -1.f}) - get_collider().get_center()); }
	if (animation.is_complete()) {
		request(NimbusState::land);
		if (change_state(NimbusState::land, Enemy::get_params("land"))) { return NIMBUS_BIND(update_land); }
	}
	return NIMBUS_BIND(update_twirl_cut);
}

fsm::StateFunction Nimbus::update_flourish() {
	p_state.actual = NimbusState::flourish;
	if (animation.is_complete()) {
		if (!m_cooldowns.post_slash.running()) { request(NimbusState::prepare_forward_slash); }
		if (change_state(NimbusState::prepare_forward_slash, Enemy::get_params("prepare_forward_slash"))) { return NIMBUS_BIND(update_prepare_forward_slash); }
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_flourish);
}

fsm::StateFunction Nimbus::update_downward_cut() {
	p_state.actual = NimbusState::downward_cut;
	if (animation.just_started()) {
		if (directions.desired != directions.actual) { request_flip(); }
	}
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 28.f, 8.f});
		i == 1 ? slash.set_constant_radius(36.f) : slash.set_constant_radius(18.f);
	}
	if (animation.get_frame_count() < 2) { get_collider().physics.zero(); }
	if (animation.get_frame_count() == 2 && animation.keyframe_started()) { get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * 28.f, 18.f}; }
	if (animation.is_complete()) {
		request(NimbusState::get_up);
		if (change_state(NimbusState::get_up, Enemy::get_params("get_up"))) { return NIMBUS_BIND(update_get_up); }
	}
	return NIMBUS_BIND(update_downward_cut);
}

fsm::StateFunction Nimbus::update_upward_cut() {
	p_state.actual = NimbusState::upward_cut;
	auto sign = directions.actual.as_float();
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{0.f, -18.f});
		i == 1 ? slash.set_constant_radius(40.f) : slash.set_constant_radius(18.f);
	}
	if (animation.is_complete()) {
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_upward_cut);
}

fsm::StateFunction Nimbus::update_turn() {
	p_state.actual = NimbusState::turn;
	directions.desired.lock();
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * -20.f, -18.f});
		i == 1 ? slash.set_constant_radius(44.f) : slash.set_constant_radius(18.f);
	}
	if (animation.is_complete()) {
		m_cooldowns.post_slash.start();
		request_flip();
		request(NimbusState::get_up);
		if (change_state(NimbusState::get_up, Enemy::get_params("get_up"))) { return NIMBUS_BIND(update_get_up); }
	}
	return NIMBUS_BIND(update_turn);
}

fsm::StateFunction Nimbus::update_parry() {
	p_state.actual = NimbusState::parry;
	animation.get_frame_count() == 0 ? flags.state.reset(StateFlags::vulnerable) : flags.state.set(StateFlags::vulnerable);
	if (change_state(NimbusState::upward_cut, Enemy::get_params("upward_cut"))) { return NIMBUS_BIND(update_upward_cut); }
	if (change_state(NimbusState::parry, Enemy::get_params("parry"))) { return NIMBUS_BIND(update_parry); }
	if (animation.is_complete()) {
		if (random::percent_chance(50)) { request(NimbusState::jump); }
		if (!m_cooldowns.post_slash.running()) { random::coin_flip() ? request(NimbusState::prepare_forward_slash) : request(NimbusState::prepare_twirl_cut); }
		if (change_state(NimbusState::prepare_forward_slash, Enemy::get_params("prepare_forward_slash"))) { return NIMBUS_BIND(update_prepare_forward_slash); }
		if (change_state(NimbusState::prepare_twirl_cut, Enemy::get_params("prepare_twirl_cut"))) { return NIMBUS_BIND(update_prepare_twirl_cut); }
		if (change_state(NimbusState::jump, Enemy::get_params("jump"))) { return NIMBUS_BIND(update_jump); }
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_parry);
}

fsm::StateFunction Nimbus::update_begin_levitate() {
	p_state.actual = NimbusState::begin_levitate;
	get_collider().set_flag(shape::ColliderFlags::simple);
	get_collider().set_flag(shape::ColliderFlags::gravity, false);
	if (animation.just_started()) { m_steer_target = m_map->get_random_home_point(); }
	if (animation.is_complete()) {
		request(NimbusState::levitate);
		if (change_state(NimbusState::levitate, Enemy::get_params("levitate"))) { return NIMBUS_BIND(update_levitate); }
	}
	return NIMBUS_BIND(update_begin_levitate);
}

fsm::StateFunction Nimbus::update_levitate() {
	p_state.actual = NimbusState::levitate;
	if (change_state(NimbusState::spellcast, Enemy::get_params("spellcast"))) { return NIMBUS_BIND(update_spellcast); }
	return NIMBUS_BIND(update_levitate);
}

fsm::StateFunction Nimbus::update_spellcast() {
	p_state.actual = NimbusState::spellcast;
	if (change_state(NimbusState::double_aerial_slash, Enemy::get_params("double_aerial_slash"))) { return NIMBUS_BIND(update_double_aerial_slash); }
	return NIMBUS_BIND(update_spellcast);
}

fsm::StateFunction Nimbus::update_double_aerial_slash() {
	p_state.actual = NimbusState::double_aerial_slash;
	get_collider().set_flag(shape::ColliderFlags::simple, false);
	get_collider().set_flag(shape::ColliderFlags::gravity);
	if (animation.get_frame_count() < 2) { get_collider().physics.zero(); }
	if (animation.is_complete()) {
		request(NimbusState::land);
		if (change_state(NimbusState::land, Enemy::get_params("land"))) { return NIMBUS_BIND(update_land); }
	}
	return NIMBUS_BIND(update_double_aerial_slash);
}

void Nimbus::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Nimbus Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("jump")) { request(NimbusState::jump); }
		if (ImGui::Button("land")) { request(NimbusState::land); }
		if (ImGui::Button("prepare_forward_slash")) { request(NimbusState::prepare_forward_slash); }
		if (ImGui::Button("prepare_twirl_cut")) { request(NimbusState::prepare_twirl_cut); }
		if (ImGui::Button("flourish")) { request(NimbusState::flourish); }
		if (ImGui::Button("downward_cut")) { request(NimbusState::downward_cut); }
		if (ImGui::Button("upward_cut")) { request(NimbusState::upward_cut); }
		if (ImGui::Button("turn")) { request(NimbusState::turn); }
		if (ImGui::Button("parry")) { request(NimbusState::parry); }
		if (ImGui::Button("begin_levitate")) { request(NimbusState::begin_levitate); }
		if (ImGui::Button("spellcast")) { request(NimbusState::spellcast); }
		if (ImGui::Button("double_aerial_slash")) { request(NimbusState::double_aerial_slash); }
		ImGui::End();
	}
}

bool Nimbus::change_state(NimbusState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
