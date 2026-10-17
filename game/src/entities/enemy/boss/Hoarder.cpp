
#include <fornani/automa/SceneContext.hpp>
#include <fornani/core/Debug.hpp>
#include <fornani/entities/enemy/boss/Hoarder.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Hoarder::Hoarder(automa::ServiceProvider& svc, world::Map& map) : Boss(svc, map, "hoarder"), m_slash_wave(svc, "slash_wave"), m_map{&map}, m_services{&svc}, m_trash_toss{svc, "trash"} {
	auto const fr = 64;
	p_animatable.set_animations({{"idle", {15, 1, fr, -1}},
								 {"burrow", {16, 5, fr, 0}},
								 {"rummage", {6, 2, fr, -1}},
								 {"emerge", {8, 1, fr, 0}},
								 {"get_up", {8, 1, fr, 0}},
								 {"bg_burrow", {4, 2, fr, 0}},
								 {"bg_rummage", {1, 2, fr, -1}},
								 {"bg_emerge", {0, 1, fr, 0}},
								 {"bg_alert", {3, 1, fr * 3, 0}},
								 {"present", {9, 1, fr * 3, 0}},
								 {"hop_on_bike", {10, 1, fr, 0}},
								 {"ride_bike", {11, 2, fr, 3}},
								 {"tantrum", {13, 2, fr, 3}},
								 {"die", {21, 2, fr, -1, true}},
								 {"run", {23, 4, fr, -1}},
								 {"laugh", {27, 2, fr, 3}},
								 {"cry", {29, 2, fr, -1}}});
	p_animatable.animation.set_params(get_params("idle"));
	get_collider().physics.set_friction_componentwise({0.95f, 0.995f});
	m_slash_wave.get().set_team(arms::Team::beast);
	m_trash_toss.get().set_team(arms::Team::beast);
	flags.general.set(GeneralFlags::has_invincible_channel);
	get_collider().set_resolution_exclusion_target(shape::ColliderTrait::block);
	get_collider().set_exclusion_target(shape::ColliderTrait::block);
	get_collider().set_attribute(shape::ColliderAttributes::no_collision);
	flags.state.set(StateFlags::vulnerable);
	m_boundary.x = std::numeric_limits<float>::max();
	for (auto const& pt : map.home_points) {
		m_boundary.x = std::min(pt.x, m_boundary.x);
		m_boundary.y = std::max(pt.x, m_boundary.x);
	}
}

void Hoarder::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Boss::update(svc, map, player);
	if (!has_flag_set(BossFlags::battle_mode) && player.get_collider().get_center().x > m_boundary.x && !health.is_dead()) { start_battle(); }
	if (consume_flag(BossFlags::start_battle)) {
		svc.music_player.load(svc.finder, "scuffle");
		svc.music_player.play_looped();
		svc.data.switch_destructible_state(141101, true);
	}
	if (has_flag_set(BossFlags::end_battle) && !has_flag_set(BossFlags::post_death)) {
		svc.data.switch_destructible_state(141101, true);
		svc.music_player.pause();
		set_flag(BossFlags::post_death);
		svc.music_player.load(svc.finder, "none");
		svc.music_player.play_looped();
		m_attacks.shockwaves.clear();
	}

	// logic
	m_slash_wave.update(svc, map, *this);
	auto bp = Enemy::get_collider().get_center();
	m_slash_wave.get().set_barrel_point(bp);
	m_trash_toss.update(svc, map, *this);
	if (m_flags.test(HoarderFlags::toss) && !health.is_dead()) {
		auto const hp = map.get_random_home_point({m_previous_home_point});
		auto bp = sf::Vector2f{directions.actual.as_float() * 10.f, -120.f};
		auto const target = get_collider().get_center() + bp;
		m_previous_home_point = hp;
		m_trash_toss.get().set_barrel_point(target);
		m_trash_toss.get().shoot(svc, map, hp - target);
		m_flags.reset(HoarderFlags::toss);
	}
	// shockwaves
	for (auto& s : m_attacks.shockwaves) {
		if (s.hit.active()) { player.hurt(); }
		s.update(svc, map);
		s.handle_player(player);
	}
	std::erase_if(m_attacks.shockwaves, [](auto const& s) { return s.lifetime.is_almost_complete(); });

	// melee attacks
	if (Boss::has_flag_set(BossFlags::battle_mode)) {}

	// animation

	if (is_hostile()) {}
	if (player.is_dead()) { request(HoarderState::cry); }

	state_function = state_function();
}

void Hoarder::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::render(svc, win, cam);
	for (auto& slash : m_attacks.slash) {
		// if (slash.hit.active()) { slash.render(win, cam); }
	}
}

void Hoarder::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	debug();
}
fsm::StateFunction Hoarder::update_idle() {
	p_state.actual = HoarderState::idle;
	if (p_animatable.animation.just_started()) {}
	if (change_state(HoarderState::burrow, Enemy::get_params("burrow"))) { return HOARDER_BIND(update_burrow); }
	if (change_state(HoarderState::rummage, Enemy::get_params("rummage"))) { return HOARDER_BIND(update_rummage); }
	if (change_state(HoarderState::emerge, Enemy::get_params("emerge"))) { return HOARDER_BIND(update_emerge); }
	if (change_state(HoarderState::bg_burrow, Enemy::get_params("bg_burrow"))) { return HOARDER_BIND(update_bg_burrow); }
	if (change_state(HoarderState::bg_rummage, Enemy::get_params("bg_rummage"))) { return HOARDER_BIND(update_bg_rummage); }
	if (change_state(HoarderState::bg_emerge, Enemy::get_params("bg_emerge"))) { return HOARDER_BIND(update_bg_emerge); }
	if (change_state(HoarderState::bg_alert, Enemy::get_params("bg_alert"))) { return HOARDER_BIND(update_bg_alert); }
	if (change_state(HoarderState::present, Enemy::get_params("present"))) { return HOARDER_BIND(update_present); }
	if (change_state(HoarderState::hop_on_bike, Enemy::get_params("hop_on_bike"))) { return HOARDER_BIND(update_hop_on_bike); }
	if (change_state(HoarderState::ride_bike, Enemy::get_params("ride_bike"))) { return HOARDER_BIND(update_ride_bike); }
	if (change_state(HoarderState::tantrum, Enemy::get_params("tantrum"))) { return HOARDER_BIND(update_tantrum); }
	if (change_state(HoarderState::die, Enemy::get_params("die"))) { return HOARDER_BIND(update_die); }
	if (change_state(HoarderState::run, Enemy::get_params("run"))) { return HOARDER_BIND(update_run); }
	if (change_state(HoarderState::laugh, Enemy::get_params("laugh"))) { return HOARDER_BIND(update_laugh); }
	if (change_state(HoarderState::cry, Enemy::get_params("cry"))) { return HOARDER_BIND(update_cry); }
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::idle);
		if (change_state(HoarderState::idle, Enemy::get_params("idle"))) { return HOARDER_BIND(update_idle); }
	}
	return HOARDER_BIND(update_idle);
}

fsm::StateFunction Hoarder::update_burrow() {
	p_state.actual = HoarderState::burrow;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::bg_emerge);
		if (change_state(HoarderState::bg_emerge, Enemy::get_params("bg_emerge"))) { return HOARDER_BIND(update_bg_emerge); }
	}
	return HOARDER_BIND(update_burrow);
}

fsm::StateFunction Hoarder::update_rummage() {
	p_state.actual = HoarderState::rummage;
	if (p_animatable.animation.just_started()) { m_flags.set(HoarderFlags::toss); }
	if (p_animatable.frame_action(0)) { m_flags.set(HoarderFlags::toss); }
	if (p_animatable.frame_action(1)) { m_flags.set(HoarderFlags::toss); }
	if (change_state(HoarderState::emerge, Enemy::get_params("emerge"))) { return HOARDER_BIND(update_emerge); }
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::idle);
		if (change_state(HoarderState::idle, Enemy::get_params("idle"))) { return HOARDER_BIND(update_idle); }
	}
	return HOARDER_BIND(update_rummage);
}

fsm::StateFunction Hoarder::update_emerge() {
	p_state.actual = HoarderState::emerge;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::idle);
		if (change_state(HoarderState::idle, Enemy::get_params("idle"))) { return HOARDER_BIND(update_idle); }
	}
	return HOARDER_BIND(update_emerge);
}

fsm::StateFunction Hoarder::update_bg_burrow() {
	p_state.actual = HoarderState::bg_burrow;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::emerge);
		if (change_state(HoarderState::emerge, Enemy::get_params("emerge"))) { return HOARDER_BIND(update_emerge); }
	}
	return HOARDER_BIND(update_bg_burrow);
}

fsm::StateFunction Hoarder::update_bg_rummage() {
	p_state.actual = HoarderState::bg_rummage;
	if (p_animatable.animation.just_started()) {}
	if (change_state(HoarderState::bg_alert, Enemy::get_params("bg_alert"))) { return HOARDER_BIND(update_bg_alert); }
	if (p_animatable.animation.is_complete()) {}
	return HOARDER_BIND(update_bg_rummage);
}

fsm::StateFunction Hoarder::update_bg_emerge() {
	p_state.actual = HoarderState::bg_emerge;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::bg_rummage);
		if (change_state(HoarderState::bg_rummage, Enemy::get_params("bg_rummage"))) { return HOARDER_BIND(update_bg_rummage); }
	}
	return HOARDER_BIND(update_bg_emerge);
}

fsm::StateFunction Hoarder::update_bg_alert() {
	p_state.actual = HoarderState::bg_alert;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::bg_burrow);
		if (change_state(HoarderState::bg_burrow, Enemy::get_params("bg_burrow"))) { return HOARDER_BIND(update_bg_burrow); }
	}
	return HOARDER_BIND(update_bg_alert);
}

fsm::StateFunction Hoarder::update_present() {
	p_state.actual = HoarderState::present;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::hop_on_bike);
		if (change_state(HoarderState::hop_on_bike, Enemy::get_params("hop_on_bike"))) { return HOARDER_BIND(update_hop_on_bike); }
	}
	return HOARDER_BIND(update_present);
}

fsm::StateFunction Hoarder::update_hop_on_bike() {
	p_state.actual = HoarderState::hop_on_bike;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::ride_bike);
		if (change_state(HoarderState::ride_bike, Enemy::get_params("ride_bike"))) { return HOARDER_BIND(update_ride_bike); }
	}
	return HOARDER_BIND(update_hop_on_bike);
}

fsm::StateFunction Hoarder::update_ride_bike() {
	p_state.actual = HoarderState::ride_bike;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::get_up);
		if (change_state(HoarderState::get_up, Enemy::get_params("get_up"))) { return HOARDER_BIND(update_get_up); }
	}
	return HOARDER_BIND(update_ride_bike);
}

fsm::StateFunction Hoarder::update_tantrum() {
	p_state.actual = HoarderState::tantrum;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::get_up);
		if (change_state(HoarderState::get_up, Enemy::get_params("get_up"))) { return HOARDER_BIND(update_get_up); }
	}
	return HOARDER_BIND(update_tantrum);
}

fsm::StateFunction Hoarder::update_die() {
	p_state.actual = HoarderState::die;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {}
	return HOARDER_BIND(update_die);
}

fsm::StateFunction Hoarder::update_run() {
	p_state.actual = HoarderState::run;
	if (p_animatable.animation.just_started()) {}
	if (change_state(HoarderState::idle, Enemy::get_params("idle"))) { return HOARDER_BIND(update_idle); }
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::idle);
		if (change_state(HoarderState::idle, Enemy::get_params("idle"))) { return HOARDER_BIND(update_idle); }
	}
	return HOARDER_BIND(update_run);
}

fsm::StateFunction Hoarder::update_laugh() {
	p_state.actual = HoarderState::laugh;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::idle);
		if (change_state(HoarderState::idle, Enemy::get_params("idle"))) { return HOARDER_BIND(update_idle); }
	}
	return HOARDER_BIND(update_laugh);
}

fsm::StateFunction Hoarder::update_cry() {
	p_state.actual = HoarderState::cry;
	if (p_animatable.animation.just_started()) {}
	if (change_state(HoarderState::idle, Enemy::get_params("idle"))) { return HOARDER_BIND(update_idle); }
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::idle);
		if (change_state(HoarderState::idle, Enemy::get_params("idle"))) { return HOARDER_BIND(update_idle); }
	}
	return HOARDER_BIND(update_cry);
}

fsm::StateFunction Hoarder::update_get_up() {
	p_state.actual = HoarderState::get_up;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.is_complete()) {
		request(HoarderState::idle);
		if (change_state(HoarderState::idle, Enemy::get_params("idle"))) { return HOARDER_BIND(update_idle); }
	}
	return HOARDER_BIND(update_get_up);
}

void Hoarder::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Hoarder Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("idle")) { request(HoarderState::idle); }
		if (ImGui::Button("burrow")) { request(HoarderState::burrow); }
		if (ImGui::Button("rummage")) { request(HoarderState::rummage); }
		if (ImGui::Button("emerge")) { request(HoarderState::emerge); }
		if (ImGui::Button("bg_burrow")) { request(HoarderState::bg_burrow); }
		if (ImGui::Button("bg_rummage")) { request(HoarderState::bg_rummage); }
		if (ImGui::Button("bg_emerge")) { request(HoarderState::bg_emerge); }
		if (ImGui::Button("bg_alert")) { request(HoarderState::bg_alert); }
		if (ImGui::Button("present")) { request(HoarderState::present); }
		if (ImGui::Button("hop_on_bike")) { request(HoarderState::hop_on_bike); }
		if (ImGui::Button("ride_bike")) { request(HoarderState::ride_bike); }
		if (ImGui::Button("tantrum")) { request(HoarderState::tantrum); }
		if (ImGui::Button("die")) { request(HoarderState::die); }
		if (ImGui::Button("run")) { request(HoarderState::run); }
		if (ImGui::Button("laugh")) { request(HoarderState::laugh); }
		if (ImGui::Button("cry")) { request(HoarderState::cry); }
		ImGui::End();
	}
}

bool Hoarder::change_state(HoarderState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
