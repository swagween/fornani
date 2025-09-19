
#include <fornani/entities/enemy/boss/Lynx.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

static bool b_lynx_start{};
static void lynx_start_battle(int battle) { b_lynx_start = true; }
constexpr auto lynx_framerate = 10;
constexpr auto run_threshold_v = 0.002f;

Lynx::Lynx(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console)
	: Enemy(svc, "lynx"), NPC(svc, "lynx"), m_health_bar(svc, "lynx"), m_params{{"sit", {0, 1, lynx_framerate, -1}},		   {"get_up", {1, 2, lynx_framerate * 4, 0}},		   {"idle", {3, 6, lynx_framerate * 4, -1}},
																				{"jump", {10, 2, lynx_framerate * 2, 0}},	   {"forward_slash", {12, 2, lynx_framerate * 4, 0}},  {"run", {14, 4, lynx_framerate * 3, -1}},
																				{"levitate", {18, 4, lynx_framerate * 4, 3}},  {"downward_slam", {22, 14, lynx_framerate * 3, 0}}, {"prepare_shuriken", {36, 3, lynx_framerate, 0}},
																				{"toss_shuriken", {39, 6, lynx_framerate, 3}}, {"triple_slash", {45, 18, lynx_framerate * 2, 0}},  {"upward_slash", {52, 5, lynx_framerate * 3, 0}},
																				{"turn", {56, 7, lynx_framerate * 2, 0}}},
	  m_console{&console}, m_cooldowns{.run{240}}, m_services{&svc} {
	Enemy::animation.set_params(get_params("sit"));
	svc.events.register_event(std::make_unique<Event<int>>("StartBattle", &lynx_start_battle));
	Enemy::collider.physics.set_friction_componentwise({0.97f, 0.99f});
}

void Lynx::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	Enemy::face_player(player);

	m_cooldowns.run.update();

	m_health_bar.update(health.get_normalized());
	if (ccm::abs(Enemy::collider.physics.acceleration.x) > run_threshold_v) { request(LynxState::run); }
	if (Enemy::directions.actual.lnr != Enemy::directions.desired.lnr) { request(LynxState::turn); }
	if (b_lynx_start) {
		m_flags.set(LynxFlags::battle_mode);
		request(LynxState::get_up);
		b_lynx_start = false;
		svc.music_player.load(svc.finder, "scuffle");
		svc.music_player.play_looped();
	}
	if (m_flags.test(LynxFlags::battle_mode)) { flags.state.set(StateFlags::vulnerable); }
	state_function = state_function();
}

void Lynx::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Enemy::render(svc, win, cam); }

void Lynx::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_flags.test(LynxFlags::battle_mode)) { m_health_bar.render(win); }
	debug();
}

fsm::StateFunction Lynx::update_sit() {
	m_state.actual = LynxState::sit;
	if (change_state(LynxState::get_up, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	if (change_state(LynxState::forward_slash, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	if (change_state(LynxState::triple_slash, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	if (change_state(LynxState::upward_slash, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	if (change_state(LynxState::downward_slam, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	return LYNX_BIND(update_sit);
}

fsm::StateFunction Lynx::update_get_up() {
	m_state.actual = LynxState::get_up;
	m_flags.set(LynxFlags::battle_mode);
	if (Enemy::animation.complete()) {
		if (change_state(LynxState::forward_slash, get_params("forward_slash"))) { return LYNX_BIND(update_forward_slash); }
		if (change_state(LynxState::triple_slash, get_params("triple_slash"))) { return LYNX_BIND(update_triple_slash); }
		if (change_state(LynxState::upward_slash, get_params("upward_slash"))) { return LYNX_BIND(update_upward_slash); }
		if (change_state(LynxState::downward_slam, get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
		if (change_state(LynxState::levitate, get_params("levitate"))) { return LYNX_BIND(update_levitate); }
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_get_up);
}

fsm::StateFunction Lynx::update_idle() {
	m_state.actual = LynxState::idle;
	if (change_state(LynxState::turn, get_params("turn"))) { return LYNX_BIND(update_turn); }
	if (change_state(LynxState::forward_slash, get_params("forward_slash"))) { return LYNX_BIND(update_forward_slash); }
	if (change_state(LynxState::triple_slash, get_params("triple_slash"))) { return LYNX_BIND(update_triple_slash); }
	if (change_state(LynxState::upward_slash, get_params("upward_slash"))) { return LYNX_BIND(update_upward_slash); }
	if (change_state(LynxState::downward_slam, get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	if (change_state(LynxState::run, get_params("run"))) { return LYNX_BIND(update_run); }
	if (change_state(LynxState::jump, get_params("jump"))) { return LYNX_BIND(update_jump); }
	if (change_state(LynxState::levitate, get_params("levitate"))) { return LYNX_BIND(update_levitate); }
	return LYNX_BIND(update_idle);
}

fsm::StateFunction Lynx::update_jump() {
	m_state.actual = LynxState::jump;
	if (Enemy::animation.just_started()) {
		Enemy::collider.physics.acceleration.y = -200.f;
		Enemy::collider.physics.acceleration.x = Enemy::directions.actual.as_float() * 60.f;
	}
	if (Enemy::animation.complete()) {
		random::percent_chance(50) ? request(LynxState::forward_slash) : request(LynxState::downward_slam);
		if (change_state(LynxState::forward_slash, get_params("forward_slash"))) { return LYNX_BIND(update_forward_slash); }
		if (change_state(LynxState::downward_slam, get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	}
	return LYNX_BIND(update_jump);
}

fsm::StateFunction Lynx::update_forward_slash() {
	m_state.actual = LynxState::forward_slash;
	flags.general.reset(GeneralFlags::gravity);
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_forward_slash);
}

fsm::StateFunction Lynx::update_levitate() {
	m_state.actual = LynxState::levitate;
	flags.general.reset(GeneralFlags::gravity);
	Enemy::collider.physics.acceleration.y = -0.3f;
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		request(LynxState::downward_slam);
		if (change_state(LynxState::downward_slam, get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	}
	return LYNX_BIND(update_levitate);
}

fsm::StateFunction Lynx::update_run() {
	m_state.actual = LynxState::run;
	Enemy::collider.physics.acceleration.x = Enemy::directions.actual.as_float() * attributes.speed;
	if (Enemy::animation.just_started()) { m_cooldowns.run.start(); }
	if (change_state(LynxState::turn, get_params("turn"))) { return LYNX_BIND(update_turn); }
	if (!m_cooldowns.run.running()) {
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_run);
}

fsm::StateFunction Lynx::update_downward_slam() {
	m_state.actual = LynxState::downward_slam;
	if (Enemy::animation.get_frame_count() == 7) { Enemy::collider.physics.acceleration.y = 200.f; }
	if (Enemy::animation.get_frame_count() >= 7 && !Enemy::collider.grounded()) { Enemy::animation.set_frame(7); }
	Enemy::animation.get_frame_count() < 8 ? flags.general.reset(GeneralFlags::gravity) : flags.general.set(GeneralFlags::gravity);
	if (Enemy::animation.complete()) {
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_downward_slam);
}

fsm::StateFunction Lynx::update_prepare_shuriken() {
	m_state.actual = LynxState::prepare_shuriken;
	if (Enemy::animation.complete()) {
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_prepare_shuriken);
}

fsm::StateFunction Lynx::update_toss_shuriken() {
	m_state.actual = LynxState::toss_shuriken;
	if (change_state(LynxState::get_up, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	return LYNX_BIND(update_toss_shuriken);
}

fsm::StateFunction Lynx::update_upward_slash() {
	m_state.actual = LynxState::upward_slash;
	flags.general.reset(GeneralFlags::gravity);
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_upward_slash);
}

fsm::StateFunction Lynx::update_triple_slash() {
	m_state.actual = LynxState::triple_slash;
	if (Enemy::animation.complete()) {
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_triple_slash);
}

fsm::StateFunction Lynx::update_turn() {
	m_state.actual = LynxState::turn;
	Enemy::directions.desired.lock();
	if (Enemy::animation.complete()) {
		request_flip();
		request(LynxState::idle);
		Enemy::animation.set_params(get_params("idle"));
		return LYNX_BIND(update_idle);
	}
	return LYNX_BIND(update_turn);
}

bool Lynx::change_state(LynxState next, anim::Parameters params) {
	if (m_state.desired == next) {
		Enemy::animation.set_params(params);
		return true;
	}
	return false;
}

anim::Parameters const& Lynx::get_params(std::string const& key) { return m_params.contains(key) ? m_params.at(key) : m_params.at("idle"); }

void Lynx::debug() {
	if (ImGui::Button("forward_slash")) { request(LynxState::forward_slash); }
	if (ImGui::Button("upward_slash")) { request(LynxState::upward_slash); }
	if (ImGui::Button("triple_slash")) { request(LynxState::triple_slash); }
	if (ImGui::Button("downward_slam")) { request(LynxState::downward_slam); }
	if (ImGui::Button("run")) { request(LynxState::run); }
	if (ImGui::Button("jump")) { request(LynxState::jump); }
	if (ImGui::Button("levitate")) { request(LynxState::levitate); }
}

} // namespace fornani::enemy
