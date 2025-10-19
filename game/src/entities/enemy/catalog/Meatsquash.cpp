#include "fornani/entities/enemy/catalog/Meatsquash.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Meatsquash::Meatsquash(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, "meatsquash"), m_services(&svc), m_map(&map) {
	animation.set_params(idle);
	collider.physics.maximum_velocity = {8.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	collider.flags.general.set(shape::General::complex);
	directions.desired.lnr = LNR::right;
	directions.actual.lnr = LNR::right;
	directions.movement.lnr = LNR::neutral;
	collider.set_top_only();

	attacks.bite.sensor.bounds.setRadius(90.f);
	attacks.bite.hit.bounds.setRadius(90.f);
	attacks.bite.origin = {0.f, 0.f};

	flags.state.reset(StateFlags::vulnerable);
}

void Meatsquash::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}
	Enemy::update(svc, map, player);

	auto bite_offset = sf::Vector2f{0.f, -98.f};
	attacks.bite.set_position(collider.get_center() + bite_offset);
	attacks.bite.update();
	attacks.bite.handle_player(player);

	animation.get_frame() == 12 ? attacks.bite.hit.activate() : attacks.bite.hit.deactivate();
	if (attacks.bite.sensor.active() && attacks.bite.hit.active() && !(player.collider.get_center().y > collider.physics.position.y)) { player.hurt(24.f); }

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::hurt);
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();

	if (is_hostile() && !hostility_triggered()) { state = MeatsquashState::chomp; }
	if (just_died()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::death); }

	state_function = state_function();
}

void Meatsquash::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (died()) { return; }
	Enemy::render(svc, win, cam);
	attacks.bite.render(win, cam);
}

fsm::StateFunction Meatsquash::update_idle() {
	animation.label = "idle";
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(MeatsquashState::chomp, chomp)) { return MEATSQUASH_BIND(update_chomp); }
	state = MeatsquashState::idle;
	return MEATSQUASH_BIND(update_idle);
};

fsm::StateFunction Meatsquash::update_chomp() {
	animation.label = "chomp";
	if (animation.complete()) {
		animation.set_params(open);
		return MEATSQUASH_BIND(update_open);
	}
	state = MeatsquashState::chomp;
	return MEATSQUASH_BIND(update_chomp);
};

fsm::StateFunction Meatsquash::update_swallow() {
	animation.label = "swallow";
	if (animation.complete()) {
		animation.set_params(open);
		return MEATSQUASH_BIND(update_open);
	}
	state = MeatsquashState::swallow;
	return MEATSQUASH_BIND(update_swallow);
}

fsm::StateFunction Meatsquash::update_open() {
	animation.label = "open";
	if (animation.complete()) {
		animation.set_params(idle);
		return MEATSQUASH_BIND(update_idle);
	}
	state = MeatsquashState::open;
	return MEATSQUASH_BIND(update_open);
}

bool Meatsquash::change_state(MeatsquashState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
