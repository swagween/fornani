#include "fornani/entities/enemy/catalog/Beamstalk.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Beamstalk::Beamstalk(automa::ServiceProvider& svc, world::Map& map, sf::Vector2<int> start_direction) : Enemy(svc, "beamstalk", false, 0, start_direction), m_services(&svc), m_map(&map), beam(svc, "green_beam"), fire_rate{4} {
	animation.set_params(idle);
	collider.physics.maximum_velocity = {8.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	collider.flags.general.set(shape::General::complex);
	beam.get().set_team(arms::Team::beast);
}

void Beamstalk::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	post_beam.update();
	flags.state.set(StateFlags::vulnerable); // always vulnerable

	// reset animation states to determine next animation state
	directions.desired.lnr = (player.collider.get_center().x < collider.get_center().x) ? LNR::left : LNR::right;
	directions.movement.lnr = collider.physics.velocity.x > 0.f ? LNR::right : LNR::left;
	Enemy::update(svc, map, player);
	beam.update(svc, map, *this);
	auto bp = collider.get_center();
	bp.x += 84.f * directions.actual.as_float();
	bp.y -= 16.f;
	beam.get().set_barrel_point(bp);

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.beamstalk.set(audio::Beamstalk::hurt);
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();

	if (is_hostile() && !hostility_triggered() && !post_beam.running()) { state = BeamstalkState::charge; }
	if (just_died()) { m_services->soundboard.flags.beamstalk.set(audio::Beamstalk::death); }

	state_function = state_function();
}

void Beamstalk::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
}

fsm::StateFunction Beamstalk::update_idle() {
	animation.label = "idle";
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(BeamstalkState::charge, charge)) { return BEAMSTALK_BIND(update_charge); }
	state = BeamstalkState::idle;
	return BEAMSTALK_BIND(update_idle);
};

fsm::StateFunction Beamstalk::update_charge() {
	animation.label = "charge";
	if (animation.get_frame() > 6) {
		if (m_services->ticker.every_x_ticks(static_cast<int>(fire_rate))) {
			m_map->spawn_projectile_at(*m_services, beam.get(), beam.get().get_barrel_point());
			collider.physics.apply_force({-beam.get().get_recoil(), 0.f});
		}
	}
	if (animation.complete()) {
		animation.set_params(shoot);
		return BEAMSTALK_BIND(update_shoot);
	}
	state = BeamstalkState::charge;
	return BEAMSTALK_BIND(update_charge);
};

fsm::StateFunction Beamstalk::update_shoot() {
	animation.label = "shoot";
	if (m_services->ticker.every_x_ticks(static_cast<int>(fire_rate))) {
		m_map->spawn_projectile_at(*m_services, beam.get(), beam.get().get_barrel_point());
		collider.physics.apply_force(beam.get().get_recoil_force());
	}
	if (animation.complete()) {
		post_beam.start();
		animation.set_params(relax);
		return BEAMSTALK_BIND(update_relax);
	}
	state = BeamstalkState::shoot;
	return BEAMSTALK_BIND(update_shoot);
}

fsm::StateFunction Beamstalk::update_relax() {
	animation.label = "relax";
	if (animation.complete()) {
		animation.set_params(idle);
		return BEAMSTALK_BIND(update_idle);
	}
	state = BeamstalkState::relax;
	return BEAMSTALK_BIND(update_relax);
}

bool Beamstalk::change_state(BeamstalkState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
