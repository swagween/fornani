
#include <fornani/automa/SceneContext.hpp>
#include <fornani/core/Debug.hpp>
#include <fornani/entities/enemy/boss/Henchman.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Henchman::Henchman(automa::ServiceProvider& svc, world::Map& map) : Boss(svc, map, "henchman"), m_slash_wave(svc, "slash_wave"), m_map{&map}, m_services{&svc} {
	auto const fr = 48;
	p_animatable.set_animations({{"idle", {0, 6, 28, 3}},
								 {"prepare_downward_cut", {6, 2, 64, 0}},
								 {"prepare_upward_cut", {11, 2, 64, 0}},
								 {"downward_cut", {8, 3, 32, 0}},
								 {"upward_cut", {13, 2, 32, 0}},
								 {"knife_toss", {15, 7, 32, 0}},
								 {"forward_slash", {22, 6, fr, 0}},
								 {"whistle", {28, 8, fr, 0}},
								 {"turn", {36, 7, 32, 0}},
								 {"jumpsquat", {43, 3, fr, 0, true}},
								 {"jump", {46, 3, fr, 0, true}},
								 {"hop", {46, 3, 32, 0, true}},
								 {"back_hop", {46, 3, 32, 0, true}},
								 {"land", {49, 2, fr, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	get_collider().physics.set_friction_componentwise({0.95f, 0.995f});
	m_slash_wave.get().set_team(arms::Team::skycorps);
	flags.general.set(GeneralFlags::has_invincible_channel);
	flags.state.set(StateFlags::vulnerable);
	m_boundary.x = std::numeric_limits<float>::max();
	for (auto const& pt : map.home_points) {
		m_boundary.x = std::min(pt.x, m_boundary.x);
		m_boundary.y = std::max(pt.x, m_boundary.x);
	}
}

void Henchman::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Boss::update(svc, map, player);
	if (!has_flag_set(BossFlags::battle_mode) && player.get_collider().get_center().x > m_boundary.x && !health.is_dead()) { start_battle(); }
	if (consume_flag(BossFlags::start_battle)) {
		svc.music_player.load(svc.finder, "scuffle");
		svc.music_player.play_looped();
		svc.data.switch_destructible_state(71001, true);
	}
	if (has_flag_set(BossFlags::end_battle) && !has_flag_set(BossFlags::post_death)) {
		svc.data.switch_destructible_state(71001, true);
		svc.music_player.pause();
		set_flag(BossFlags::post_death);
		svc.music_player.load(svc.finder, "none");
		svc.music_player.play_looped();
		m_attacks.shockwaves.clear();
		map.clear_enemies({45});
	}

	// logic
	m_cooldowns.post_slash.update();
	m_cooldowns.post_whistle.update();
	m_cooldowns.post_cut.update();
	m_slash_wave.update(svc, map, *this);
	auto bp = Enemy::get_collider().get_center();
	m_slash_wave.get().set_barrel_point(bp);

	// shockwaves
	for (auto& s : m_attacks.shockwaves) {
		if (s.hit.active()) { player.hurt(); }
		s.update(svc, map);
		s.handle_player(player);
	}
	std::erase_if(m_attacks.shockwaves, [](auto const& s) { return s.lifetime.is_almost_complete(); });

	// melee attacks
	if (Boss::has_flag_set(BossFlags::battle_mode)) {
		for (auto& slash : m_attacks.slash) {
			auto damage = 1.f;
			slash.disable();
			slash.update();
			if (is_state(HenchmanState::upward_cut)) {
				slash.enable();
				if (p_animatable.animation.get_frame() != 13) { slash.disable(); }
				damage = 2.f;
			}
			if (is_state(HenchmanState::turn)) {
				slash.enable();
				if (p_animatable.animation.get_frame_count() != 4) { slash.disable(); }
				damage = 2.f;
			}
			if (is_state(HenchmanState::forward_slash)) {
				slash.enable();
				if (p_animatable.animation.get_frame_count() != 2) { slash.disable(); }
				damage = 2.f;
			}
			if (is_state(HenchmanState::downward_cut)) {
				slash.enable();
				if (p_animatable.animation.get_frame() != 8) { slash.disable(); }
				damage = 2.f;
			}
			slash.hurt_player(player, damage, {directions.desired.as_float() * 0.4f, -0.2f});
			slash.cancel_projectiles(svc, map, get_team());
			for (auto& e : map.enemy_catalog.enemies) {
				if (e.get() == this) { continue; }
				if (slash.hit.within_bounds(e->get_collider()) && slash.hit.active()) { e->kill(svc, map); }
			}
		}
	}

	// animation
	if (m_caution.is_projectile_detected(map, physical.alert_range, arms::Team::skycorps)) {
		auto const at_edge = get_collider().get_center().x < m_boundary.x || get_collider().get_center().x > m_boundary.y;
		at_edge ? request(HenchmanState::jumpsquat) : request(HenchmanState::hop);
	}
	if (is_hostile()) {
		if (!m_cooldowns.post_cut.running()) {
			request(HenchmanState::prepare_downward_cut);
		} else {
			request(HenchmanState::jumpsquat);
		}
		if (player.get_collider().get_center().y < get_collider().get_center().y) { request(HenchmanState::prepare_upward_cut); }
		if (!m_cooldowns.post_whistle.running() && random::coin_flip()) { request(HenchmanState::whistle); }
	}
	if (directions.actual.lnr != directions.desired.lnr && !is_airborne()) { request(HenchmanState::turn); }
	if (get_collider().has_left_wallslide_collision() || get_collider().has_right_wallslide_collision()) { request(HenchmanState::back_hop); }
	if (player.is_dead()) { request(HenchmanState::idle); }

	state_function = state_function();
}

void Henchman::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::render(svc, win, cam);
	for (auto& slash : m_attacks.slash) {
		// if (slash.hit.active()) { slash.render(win, cam); }
	}
}

void Henchman::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	// debug();
}

fsm::StateFunction Henchman::update_idle() {
	p_state.actual = HenchmanState::idle;
	if (has_flag_set(BossFlags::battle_mode)) {
		if (change_state(HenchmanState::prepare_downward_cut, Enemy::get_params("prepare_downward_cut"))) { return HENCHMAN_BIND(update_prepare_downward_cut); }
		if (change_state(HenchmanState::prepare_upward_cut, Enemy::get_params("prepare_upward_cut"))) { return HENCHMAN_BIND(update_prepare_upward_cut); }
		if (change_state(HenchmanState::whistle, Enemy::get_params("whistle"))) { return HENCHMAN_BIND(update_whistle); }
		if (change_state(HenchmanState::turn, Enemy::get_params("turn"))) { return HENCHMAN_BIND(update_turn); }
		if (change_state(HenchmanState::knife_toss, Enemy::get_params("knife_toss"))) { return HENCHMAN_BIND(update_knife_toss); }
		if (change_state(HenchmanState::jumpsquat, Enemy::get_params("jumpsquat"))) { return HENCHMAN_BIND(update_jumpsquat); }
		if (change_state(HenchmanState::jump, Enemy::get_params("jumpsquat"))) { return HENCHMAN_BIND(update_jumpsquat); }
		if (change_state(HenchmanState::back_hop, Enemy::get_params("back_hop"))) { return HENCHMAN_BIND(update_back_hop); }
		if (change_state(HenchmanState::hop, Enemy::get_params("hop"))) { return HENCHMAN_BIND(update_hop); }
	}
	if (p_animatable.animation.is_complete() && get_collider().grounded()) {
		if (!has_flag_set(BossFlags::battle_mode)) {
			request(HenchmanState::idle);
			if (change_state(HenchmanState::idle, Enemy::get_params("idle"))) { return HENCHMAN_BIND(update_idle); }
		}
		request(HenchmanState::jumpsquat);
		if (change_state(HenchmanState::jumpsquat, Enemy::get_params("jumpsquat"))) { return HENCHMAN_BIND(update_jumpsquat); }
	}
	return HENCHMAN_BIND(update_idle);
}

fsm::StateFunction Henchman::update_jump() {
	p_state.actual = HenchmanState::jump;
	if (p_animatable.animation.just_started()) { get_collider().physics.velocity = sf::Vector2f{0.f, -60.f}; }
	if (!get_collider().has_left_wallslide_collision() && !get_collider().has_right_wallslide_collision()) { get_collider().physics.velocity.x = directions.actual.as_float() * 12.f; }
	if (p_animatable.animation.is_complete() && get_collider().grounded()) {
		request(HenchmanState::land);
		if (change_state(HenchmanState::land, Enemy::get_params("land"))) { return HENCHMAN_BIND(update_land); }
	}
	return HENCHMAN_BIND(update_jump);
}

fsm::StateFunction Henchman::update_land() {
	p_state.actual = HenchmanState::land;
	if (p_animatable.animation.just_started()) {
		m_services->camera_controller.shake(10, 0.3f, 200, 20);
		m_services->soundboard.play_sound("vibration", get_collider().get_center());
		m_services->soundboard.play_sound("delay_crash", get_collider().get_center());
		auto const shockwave_speed = 1.2f;
		m_attacks.shockwaves.push_back(entity::Shockwave{{30, 600, 3, {shockwave_speed * directions.actual.as_float(), 0.f}}});
		m_attacks.shockwaves.back().origin = Enemy::get_collider().get_bottom();
		m_attacks.shockwaves.back().start();
		m_attacks.shockwaves.push_back(entity::Shockwave{{30, 600, 3, {-shockwave_speed * directions.actual.as_float(), 0.f}}});
		m_attacks.shockwaves.back().origin = Enemy::get_collider().get_bottom();
		m_attacks.shockwaves.back().start();
	}
	if (p_animatable.animation.is_complete()) {
		if (m_flags.consume(HenchmanFlags::retreated) && !m_cooldowns.post_whistle.running()) {
			request(HenchmanState::whistle);
			if (change_state(HenchmanState::whistle, Enemy::get_params("whistle"))) { return HENCHMAN_BIND(update_whistle); }
		}
		request(HenchmanState::idle);
		if (change_state(HenchmanState::idle, Enemy::get_params("idle"))) { return HENCHMAN_BIND(update_idle); }
	}
	return HENCHMAN_BIND(update_land);
}

fsm::StateFunction Henchman::update_hop() {
	p_state.actual = HenchmanState::hop;
	if (p_animatable.animation.just_started()) { get_collider().physics.velocity = sf::Vector2f{0.f, -40.f}; }
	if (!get_collider().has_left_wallslide_collision() && !get_collider().has_right_wallslide_collision()) { get_collider().physics.velocity.x = directions.actual.as_float() * 8.f; }
	if (p_animatable.animation.is_complete() && get_collider().grounded()) {
		request(HenchmanState::land);
		if (change_state(HenchmanState::land, Enemy::get_params("land"))) { return HENCHMAN_BIND(update_land); }
	}
	return HENCHMAN_BIND(update_hop);
}

fsm::StateFunction Henchman::update_back_hop() {
	p_state.actual = HenchmanState::back_hop;
	m_flags.set(HenchmanFlags::retreated);
	if (p_animatable.animation.just_started()) { get_collider().physics.velocity = sf::Vector2f{0.f, -40.f}; }
	get_collider().physics.velocity.x = directions.actual.as_float() * -12.f;
	if (p_animatable.animation.is_complete() && get_collider().grounded()) {
		request(HenchmanState::land);
		if (change_state(HenchmanState::land, Enemy::get_params("land"))) { return HENCHMAN_BIND(update_land); }
	}
	return HENCHMAN_BIND(update_back_hop);
}

fsm::StateFunction Henchman::update_jumpsquat() {
	p_state.actual = HenchmanState::jumpsquat;
	if (p_animatable.animation.is_complete()) {
		request(HenchmanState::jump);
		if (change_state(HenchmanState::jump, Enemy::get_params("jump"))) { return HENCHMAN_BIND(update_jump); }
	}
	return HENCHMAN_BIND(update_jumpsquat);
}

fsm::StateFunction Henchman::update_forward_slash() {
	p_state.actual = HenchmanState::forward_slash;
	if (p_animatable.animation.get_frame_count() == 2 && p_animatable.animation.keyframe_started()) { m_services->soundboard.play_sound("nimbus_swipe_1", get_collider().get_center()); }
	if (p_animatable.frame_action(2)) { get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * 40.f, 0.f}; }
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * (318.f - 60.f * i), -20.f * i + 26.f});
		i == 1 ? slash.set_constant_radius(44.f) : i == 0 ? slash.set_constant_radius(36.f) : slash.set_constant_radius(40.f);
	}
	if (p_animatable.animation.is_complete()) {
		flags.state.set(StateFlags::vulnerable);
		m_cooldowns.post_slash.start();
		if (change_state(HenchmanState::turn, Enemy::get_params("turn"))) { return HENCHMAN_BIND(update_turn); }
		request(HenchmanState::idle);
		if (change_state(HenchmanState::idle, Enemy::get_params("idle"))) { return HENCHMAN_BIND(update_idle); }
	}
	return HENCHMAN_BIND(update_forward_slash);
}

fsm::StateFunction Henchman::update_whistle() {
	p_state.actual = HenchmanState::whistle;
	if (p_animatable.animation.get_frame_count() == 2 && p_animatable.animation.keyframe_started()) { m_services->soundboard.play_sound("haunch_whistle", get_collider().get_center()); }
	if (p_animatable.animation.get_frame_count() == 5 && !m_cooldowns.post_whistle.running() && has_flag_set(BossFlags::battle_mode)) {
		for (auto i = 0; i < 2; ++i) {
			auto pos = get_collider().get_top() + sf::Vector2f{0.f, -140.f} + random::random_vector_float(-200.f, 200.f);
			m_map->spawn_enemy(4, pos);
		}
		m_cooldowns.post_whistle.start();
	}
	if (p_animatable.animation.is_complete()) {
		request(HenchmanState::idle);
		if (change_state(HenchmanState::idle, Enemy::get_params("idle"))) { return HENCHMAN_BIND(update_idle); }
	}
	return HENCHMAN_BIND(update_whistle);
}

fsm::StateFunction Henchman::update_knife_toss() {
	p_state.actual = HenchmanState::knife_toss;
	if (p_animatable.animation.is_complete()) {
		request(HenchmanState::forward_slash);
		if (change_state(HenchmanState::forward_slash, Enemy::get_params("forward_slash"))) { return HENCHMAN_BIND(update_forward_slash); }
	}
	return HENCHMAN_BIND(update_knife_toss);
}

fsm::StateFunction Henchman::update_prepare_downward_cut() {
	p_state.actual = HenchmanState::prepare_downward_cut;
	if (p_animatable.animation.is_complete()) {
		request(HenchmanState::downward_cut);
		if (change_state(HenchmanState::downward_cut, Enemy::get_params("downward_cut"))) { return HENCHMAN_BIND(update_downward_cut); }
	}
	return HENCHMAN_BIND(update_prepare_downward_cut);
}

fsm::StateFunction Henchman::update_prepare_upward_cut() {
	p_state.actual = HenchmanState::prepare_upward_cut;
	if (p_animatable.animation.is_complete()) {
		request(HenchmanState::upward_cut);
		if (change_state(HenchmanState::upward_cut, Enemy::get_params("upward_cut"))) { return HENCHMAN_BIND(update_upward_cut); }
	}
	return HENCHMAN_BIND(update_prepare_upward_cut);
}

fsm::StateFunction Henchman::update_downward_cut() {
	p_state.actual = HenchmanState::downward_cut;
	if (p_animatable.animation.just_started()) { m_services->soundboard.play_sound("nimbus_swipe_1", get_collider().get_center()); }
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 100.f, -120.f + 80.f * i});
		i == 1 ? slash.set_constant_radius(120.f) : slash.set_constant_radius(8.f);
	}
	if (p_animatable.animation.is_complete()) {
		m_cooldowns.post_cut.start();
		request(HenchmanState::idle);
		if (change_state(HenchmanState::idle, Enemy::get_params("idle"))) { return HENCHMAN_BIND(update_idle); }
	}
	return HENCHMAN_BIND(update_downward_cut);
}

fsm::StateFunction Henchman::update_upward_cut() {
	p_state.actual = HenchmanState::upward_cut;
	if (p_animatable.animation.just_started()) { m_services->soundboard.play_sound("nimbus_swipe_3", get_collider().get_center()); }
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		auto const xoff = i == 1 ? 36.f : 0.f;
		auto const yoff = i == 1 ? 20.f : 0.f;
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * (80.f + xoff), -200.f + 80.f * i - yoff});
		i == 1 ? slash.set_constant_radius(100.f) : slash.set_constant_radius(60.f);
	}
	if (p_animatable.animation.is_complete()) {
		request(HenchmanState::idle);
		if (change_state(HenchmanState::idle, Enemy::get_params("idle"))) { return HENCHMAN_BIND(update_idle); }
	}
	return HENCHMAN_BIND(update_upward_cut);
}

fsm::StateFunction Henchman::update_turn() {
	p_state.actual = HenchmanState::turn;
	directions.desired.lock();
	if (p_animatable.animation.get_frame_count() == 4 && p_animatable.animation.keyframe_started()) { m_services->soundboard.play_sound("nimbus_swipe_3", get_collider().get_center()); }
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		auto const xoff = i == 1 ? -64.f : 0.f;
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * (-60.f + xoff), -20.f - 60.f * i});
		i == 1 ? slash.set_constant_radius(80.f) : slash.set_constant_radius(40.f);
	}
	if (p_animatable.animation.is_complete()) {
		flags.state.set(StateFlags::vulnerable);
		m_cooldowns.post_slash.start();
		request_flip();
		request(HenchmanState::knife_toss);
		if (change_state(HenchmanState::knife_toss, Enemy::get_params("knife_toss"))) { return HENCHMAN_BIND(update_knife_toss); }
	}
	return HENCHMAN_BIND(update_turn);
}

void Henchman::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Henchman Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("jump")) { request(HenchmanState::jump); }
		if (ImGui::Button("land")) { request(HenchmanState::land); }
		if (ImGui::Button("whistle")) { request(HenchmanState::whistle); }
		if (ImGui::Button("knife toss")) { request(HenchmanState::knife_toss); }
		if (ImGui::Button("downward_cut")) { request(HenchmanState::prepare_downward_cut); }
		if (ImGui::Button("upward_cut")) { request(HenchmanState::prepare_upward_cut); }
		if (ImGui::Button("hop")) { request(HenchmanState::hop); }
		ImGui::End();
	}
}

bool Henchman::change_state(HenchmanState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
