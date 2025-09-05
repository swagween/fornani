
#include <fornani/entities/enemy/boss/Miaag.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

static bool b_miaag_start{};
static void miaag_start_battle(int battle) { b_miaag_start = true; }

Miaag::Miaag(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, "miaag"), m_health_bar(svc, "miaag"), m_params{{"idle", {0, 7, 40, -1}}, {"chomp", {7, 9, 40, 0}}, {"turn", {16, 1, 40, 0}}, {"closed", {15, 1, 40, -1}}, {"awaken", {17, 4, 40, 0}}, {"dormant", {17, 1, 40, -1}}} {
	Enemy::animation.set_params(get_params("dormant"));
	svc.events.register_event(std::make_unique<Event<int>>("StartBattle", &miaag_start_battle));
}

void Miaag::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	face_player(player);
	// seek_home(map);
	m_health_bar.update(health.get_normalized());
	if (directions.actual.lnr != directions.desired.lnr) { request(MiaagState::turn); }
	if (b_miaag_start) {
		m_flags.set(MiaagFlags::battle_mode);
		request(MiaagState::awaken);
		b_miaag_start = false;
		svc.music_player.load(svc.finder, "scuffle");
		svc.music_player.play_looped();
	}
	state_function = state_function();
}

void Miaag::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Enemy::render(svc, win, cam); }

void Miaag::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_flags.test(MiaagFlags::battle_mode)) { m_health_bar.render(win); }
}

fsm::StateFunction Miaag::update_dormant() {
	m_state.actual = MiaagState::dormant;
	if (change_state(MiaagState::awaken, get_params("awaken"))) { return MIAAG_BIND(update_awaken); }
	return MIAAG_BIND(update_dormant);
}

fsm::StateFunction Miaag::update_awaken() {
	m_state.actual = MiaagState::awaken;
	if (animation.complete()) {
		request(MiaagState::idle);
		flags.state.set(StateFlags::vulnerable);
		if (change_state(MiaagState::idle, get_params("idle"))) { return MIAAG_BIND(update_idle); }
	}
	return MIAAG_BIND(update_awaken);
}

fsm::StateFunction Miaag::update_idle() {
	m_state.actual = MiaagState::idle;
	if (change_state(MiaagState::turn, get_params("turn"))) { return MIAAG_BIND(update_turn); }
	return MIAAG_BIND(update_idle);
}

fsm::StateFunction Miaag::update_hurt() {
	m_state.actual = MiaagState::hurt;
	return MIAAG_BIND(update_hurt);
}

fsm::StateFunction Miaag::update_closed() {
	m_state.actual = MiaagState::closed;
	return MIAAG_BIND(update_closed);
}

fsm::StateFunction Miaag::update_chomp() {
	m_state.actual = MiaagState::chomp;
	return MIAAG_BIND(update_chomp);
}

fsm::StateFunction Miaag::update_turn() {
	m_state.actual = MiaagState::turn;
	directions.desired.lock();
	if (animation.complete()) {
		request_flip();
		request(MiaagState::idle);
		animation.set_params(get_params("idle"));
		return MIAAG_BIND(update_idle);
	}
	return MIAAG_BIND(update_turn);
}

bool Miaag::change_state(MiaagState next, anim::Parameters params) {
	if (m_state.desired == next) {
		Enemy::animation.set_params(params);
		return true;
	}
	return false;
}

anim::Parameters const& Miaag::get_params(std::string const& key) { return m_params.contains(key) ? m_params.at(key) : m_params.at("idle"); }

} // namespace fornani::enemy
