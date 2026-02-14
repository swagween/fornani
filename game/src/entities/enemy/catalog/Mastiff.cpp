
#include <fornani/entities/enemy/catalog/Mastiff.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto mastiff_framerate = 10;

Mastiff::Mastiff(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "mastiff"), m_services{&svc} {
	m_params = {{"idle", {0, 6, mastiff_framerate * 2, -1}}, {"run", {6, 4, mastiff_framerate * 2, 4}}, {"bite", {10, 6, mastiff_framerate * 2, 0}}, {"turn", {16, 2, mastiff_framerate * 2, 0}}};
	animation.set_params(get_params("idle"));
	m_bite.hit.bounds.setRadius(40.f);

	get_collider().physics.set_friction_componentwise({0.92f, 0.99f});
}

void Mastiff::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }
	face_player(player);
	flags.state.set(StateFlags::vulnerable);

	if (is_hostile()) { request(MastiffState::bite); }

	m_bite.hit.deactivate();
	m_bite.hit.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 32.f, -18.f});

	if (svc.ticker.every_second()) {
		if (random::percent_chance(30)) { request(MastiffState::run); }
	}

	// caution
	auto incoming_projectile = m_caution.projectile_detected(map, physical.alert_range, arms::Team::guardian);
	if (incoming_projectile.lnr != LNR::neutral) {
		if (incoming_projectile.lnr != directions.actual.lnr) { request(MastiffState::bite); }
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(MastiffState::turn); }

	state_function = state_function();

	if (m_bite.hit.within_bounds(player.get_collider().hurtbox) && m_bite.hit.active() && !health.is_dead()) { player.hurt(); }
}

void Mastiff::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (svc.greyblock_mode()) { m_bite.render(win, cam); }
}

fsm::StateFunction Mastiff::update_idle() {
	p_state.actual = MastiffState::idle;
	if (change_state(MastiffState::turn, get_params("turn"))) { return MASTIFF_BIND(update_turn); }
	if (change_state(MastiffState::run, get_params("run"))) { return MASTIFF_BIND(update_run); }
	if (change_state(MastiffState::bite, get_params("bite")) && get_collider().grounded()) { return MASTIFF_BIND(update_bite); }
	return MASTIFF_BIND(update_idle);
}

fsm::StateFunction Mastiff::update_run() {
	p_state.actual = MastiffState::run;
	get_collider().physics.acceleration.x = directions.actual.as_float() * attributes.speed;
	if (animation.is_complete()) {
		if (change_state(MastiffState::bite, get_params("bite")) && get_collider().grounded()) { return MASTIFF_BIND(update_bite); }
		if (change_state(MastiffState::turn, get_params("turn"))) { return MASTIFF_BIND(update_turn); }
		request(MastiffState::idle);
		if (change_state(MastiffState::idle, get_params("idle"))) { return MASTIFF_BIND(update_idle); }
	}
	return MASTIFF_BIND(update_run);
}

fsm::StateFunction Mastiff::update_bite() {
	p_state.actual = MastiffState::bite;
	if (animation.just_started()) { m_services->soundboard.flags.mastiff.set(audio::Mastiff::growl); }
	get_collider().physics.acceleration.x = directions.actual.as_float() * 5.f;
	if (animation.get_frame_count() == 4) {
		m_bite.hit.activate();
		if (animation.keyframe_started()) { m_services->soundboard.flags.mastiff.set(audio::Mastiff::bite); }
	}
	if (animation.is_complete()) {
		request(MastiffState::idle);
		if (change_state(MastiffState::idle, get_params("idle"))) { return MASTIFF_BIND(update_idle); }
	}
	return MASTIFF_BIND(update_bite);
}

fsm::StateFunction Mastiff::update_turn() {
	p_state.actual = MastiffState::turn;
	if (animation.complete()) {
		request_flip();
		if (change_state(MastiffState::bite, get_params("bite")) && get_collider().grounded()) { return MASTIFF_BIND(update_bite); }
		request(MastiffState::idle);
		if (change_state(MastiffState::idle, get_params("idle"))) { return MASTIFF_BIND(update_idle); }
	}
	return MASTIFF_BIND(update_turn);
}

bool Mastiff::change_state(MastiffState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
