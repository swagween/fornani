
#include <fornani/entities/enemy/catalog/Hellion.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Hellion::Hellion(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "hellion"), m_services(&svc), m_map(&map), m_poison(svc, "demon_ball") {
	p_animatable.set_animations({{"run", {0, 12, 10, -1}}, {"sleep", {25, 1, 48, -1}}, {"awaken", {26, 15, 16, 0}}, {"turn", {12, 13, 20, 0}}});
	p_animatable.animation.set_params(get_params("sleep"));
	get_collider().physics.set_friction_componentwise({0.98f, 0.999f});
	flags.state.set(StateFlags::vulnerable);
	m_poison.get().set_team(arms::Team::guardian);
	attributes.team = arms::Team::guardian;
}

void Hellion::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { m_services->soundboard.play_sound("hellion_death", get_collider().get_center()); }
	Enemy::update(svc, map, player);
	if (died()) { return; }

	hurt_effect.update();
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) {
			svc.soundboard.play_sound("hellion_hurt");
			svc.soundboard.play_sound("hit_squeak");
		}
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
	}

	m_target = player.get_collider().get_center() + sf::Vector2f{0.f, -20.f} - m_poison.barrel_point();
	m_poison.update(svc, map, *this, true);
	auto bp = get_collider().get_center() + sf::Vector2f{0.f, -30.f};
	m_poison.get().set_barrel_point(bp);

	if (directions.actual.lnr != directions.desired.lnr) { request(HellionState::turn); }

	state_function = state_function();
}

void Hellion::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
}

fsm::StateFunction Hellion::update_run() {
	p_state.actual = HellionState::run;
	m_services->soundboard.repeat_sound("hellion_run", get_stable_id().get(), get_collider().get_center());
	get_collider().physics.velocity.x = directions.actual.as_float() * attributes.speed;
	if (change_state(HellionState::turn, get_params("turn"))) { return HELLION_BIND(update_turn); }
	return HELLION_BIND(update_run);
}

fsm::StateFunction Hellion::update_sleep() {
	p_state.actual = HellionState::sleep;
	if (is_hostile()) {
		request(HellionState::awaken);
		m_services->soundboard.play_sound("hellion_chirp", get_collider().get_center());
	}
	if (change_state(HellionState::awaken, get_params("awaken"))) { return HELLION_BIND(update_awaken); }
	return HELLION_BIND(update_sleep);
}

fsm::StateFunction Hellion::update_awaken() {
	p_state.actual = HellionState::awaken;
	if (p_animatable.animation.is_complete()) {
		if (change_state(HellionState::turn, get_params("turn"))) { return HELLION_BIND(update_turn); }
		request(HellionState::run);
		if (change_state(HellionState::run, get_params("run"))) { return HELLION_BIND(update_run); }
	}
	return HELLION_BIND(update_awaken);
}

fsm::StateFunction Hellion::update_turn() {
	p_state.actual = HellionState::turn;
	if (p_animatable.animation.just_started()) { m_services->soundboard.play_sound("hellion_purr", get_collider().get_center()); }
	if (p_animatable.animation.get_frame_count() == 10 && p_animatable.animation.keyframe_started()) { m_poison.shoot(*m_services, *m_map, m_target); }
	if (p_animatable.animation.complete()) {
		request_flip();
		request(HellionState::run);
		if (change_state(HellionState::run, get_params("run"))) { return HELLION_BIND(update_run); }
	}
	return HELLION_BIND(update_turn);
}

bool Hellion::change_state(HellionState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
