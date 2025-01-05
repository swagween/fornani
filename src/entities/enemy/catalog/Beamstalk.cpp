#include "Beamstalk.hpp"
#include "../../../level/Map.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Beamstalk::Beamstalk(automa::ServiceProvider& svc, world::Map& map, sf::Vector2<int> start_direction) : Enemy(svc, "beamstalk", false, 0, start_direction), m_services(&svc), m_map(&map), beam(svc, 5), fire_rate{4} {
	animation.set_params(idle);
	collider.physics.maximum_velocity = {8.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	collider.flags.general.set(shape::General::complex);
	beam.get().set_team(arms::Team::beast);
}

void Beamstalk::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	svc.soundboard.play(svc, svc.assets.b_heavy_move, 0.f, 100.f, 64, 5.f, player.collider.get_center() - collider.get_center());

	post_beam.update();
	flags.state.set(StateFlags::vulnerable); // always vulnerable

	// reset animation states to determine next animation state
	directions.desired.lr = (player.collider.get_center().x < collider.get_center().x) ? dir::LR::left : dir::LR::right;
	directions.movement.lr = collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;
	if (directions.actual.lr == dir::LR::right && visual.sprite.getScale() == sf::Vector2<float>{1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	if (directions.actual.lr == dir::LR::left && visual.sprite.getScale() == sf::Vector2<float>{-1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	Enemy::update(svc, map, player);
	beam.update(svc, map, *this);
	auto bp = collider.get_center();
	bp.x += 84.f * directions.actual.as_float();
	bp.y -= 16.f;
	beam.get().set_barrel_point(bp);

	if(flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.beamstalk.set(audio::Beamstalk::hurt);
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();

	if (hostile() && !hostility_triggered() && !post_beam.running()) { state = BeamstalkState::charge; }
	if (just_died()) { m_services->soundboard.flags.beamstalk.set(audio::Beamstalk::death); }

	state_function = state_function();
}

void Beamstalk::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
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
		if (m_services->ticker.every_x_ticks(fire_rate)) {
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
	if (m_services->ticker.every_x_ticks(fire_rate)) {
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

} // namespace enemy