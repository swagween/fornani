#include "fornani/entities/enemy/catalog/Beamstalk.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Beamstalk::Beamstalk(automa::ServiceProvider& svc, world::Map& map, sf::Vector2<int> start_direction) : Enemy(svc, map, "beamstalk", false, 0, start_direction), m_services(&svc), m_map(&map), beam(svc, "green_beam"), fire_rate{24} {

	m_params = {{"idle", {14, 14, 36, -1}}, {"charge", {0, 9, 24, 0}}, {"shoot", {9, 3, 24, 2}}, {"relax", {12, 2, 36, 0}}};

	animation.set_params(get_params("idle"));
	get_collider().physics.maximum_velocity = {8.f, 12.f};
	get_collider().physics.air_friction = {0.95f, 0.999f};
	flags.general.reset(GeneralFlags::gravity);
	get_collider().set_flag(shape::ColliderFlags::simple);
	get_collider().set_attribute(shape::ColliderAttributes::sturdy);

	beam.get().set_team(arms::Team::beast);
	if (start_direction.lengthSquared() == 0) { start_direction.x = -1; } // default to facing left
	directions.actual = Direction(start_direction);
	set_root(map);
	init.start();
}

void Beamstalk::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	init.update();
	if (init.is_almost_complete()) { set_root(map); }
	if (init.running()) { return; }
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}
	post_beam.update();
	flags.state.set(StateFlags::vulnerable); // always vulnerable

	Enemy::update(svc, map, player);
	auto offset = get_collider().dimensions * 0.5f + sf::Vector2f{-40.f * directions.actual.as_float(), 0.f};
	m_steering.seek(get_collider().physics, *m_root - offset, 0.001f);

	beam.update(svc, map, *this);
	auto bp = get_collider().get_center();
	bp.x += 84.f * directions.actual.as_float();
	bp.y -= 16.f;
	beam.get().set_barrel_point(bp);

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.beast.set(audio::Beast::hurt);
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();

	if (is_hostile() && !hostility_triggered() && !post_beam.running()) { request(BeamstalkState::charge); }
	if (just_died()) { m_services->soundboard.flags.beamstalk.set(audio::Beamstalk::death); }

	state_function = state_function();
}

void Beamstalk::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
}

fsm::StateFunction Beamstalk::update_idle() {
	animation.label = "idle";
	p_state.actual = BeamstalkState::idle;
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(BeamstalkState::charge, get_params("charge"))) { return BEAMSTALK_BIND(update_charge); }
	return BEAMSTALK_BIND(update_idle);
};

fsm::StateFunction Beamstalk::update_charge() {
	animation.label = "charge";
	p_state.actual = BeamstalkState::charge;
	if (animation.get_frame() > 6) {
		if (m_services->ticker.every_x_ticks(static_cast<int>(fire_rate))) {
			m_map->spawn_projectile_at(*m_services, beam.get(), beam.get().get_barrel_point());
			get_collider().physics.apply_force({-beam.get().get_recoil(), 0.f});
			if (!has_flag_set(BeamstalkFlags::spit)) {
				m_services->soundboard.flags.beast.set(audio::Beast::growl);
				set_flag(BeamstalkFlags::spit);
			}
		}
	}
	if (animation.complete()) {
		set_flag(BeamstalkFlags::spit, false);
		request(BeamstalkState::shoot);
		if (change_state(BeamstalkState::shoot, get_params("shoot"))) { return BEAMSTALK_BIND(update_shoot); }
	}
	return BEAMSTALK_BIND(update_charge);
};

fsm::StateFunction Beamstalk::update_shoot() {
	animation.label = "shoot";
	p_state.actual = BeamstalkState::shoot;
	if (m_services->ticker.every_x_ticks(static_cast<int>(fire_rate))) {
		m_map->spawn_projectile_at(*m_services, beam.get(), beam.get().get_barrel_point());
		get_collider().physics.apply_force(beam.get().get_recoil_force());
	}
	if (animation.complete()) {
		post_beam.start();
		request(BeamstalkState::relax);
		if (change_state(BeamstalkState::relax, get_params("relax"))) { return BEAMSTALK_BIND(update_relax); }
	}
	return BEAMSTALK_BIND(update_shoot);
}

fsm::StateFunction Beamstalk::update_relax() {
	animation.label = "relax";
	p_state.actual = BeamstalkState::relax;
	if (animation.complete()) {
		request(BeamstalkState::idle);
		if (change_state(BeamstalkState::idle, get_params("idle"))) { return BEAMSTALK_BIND(update_idle); }
	}
	return BEAMSTALK_BIND(update_relax);
}

bool Beamstalk::change_state(BeamstalkState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

void Beamstalk::set_root(world::Map& map) {
	auto closest = std::numeric_limits<float>::max();
	for (auto& vine : map.get_entities<Vine>()) {
		for (auto& link : vine->get_chain().links) {
			auto distance = (get_collider().get_center() - link.get_anchor()).length();
			if (distance < closest) {
				m_root = &link.get_anchor();
				closest = distance;
			}
		}
	}
}

} // namespace fornani::enemy
