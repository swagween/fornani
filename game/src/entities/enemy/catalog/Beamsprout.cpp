
#include <fornani/entities/enemy/catalog/Beamsprout.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Beamsprout::Beamsprout(automa::ServiceProvider& svc, world::Map& map, sf::Vector2<int> start_direction)
	: Enemy(svc, map, "beamsprout", false, 0, start_direction), m_services(&svc), m_map(&map), beam(svc, "poison_ball"), fire_rate{72}, post_beam{336} {

	m_params = {{"idle", {0, 12, 28, -1}}, {"charge", {12, 10, 20, 0}}, {"shoot", {22, 3, 20, 2}}, {"relax", {25, 1, 20, 0}}, {"turn", {26, 2, 32, 0}}};

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

void Beamsprout::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	init.update();
	if (init.is_almost_complete()) { set_root(map); }
	if (init.running()) { return; }
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}
	post_beam.update();
	face_player(player);
	flags.state.set(StateFlags::vulnerable); // always vulnerable

	Enemy::update(svc, map, player);
	auto offset = get_collider().dimensions * 0.5f + sf::Vector2f{-20.f * directions.actual.as_float(), -64.f};
	m_steering.seek(get_collider().physics, m_root->get_bob() - offset, 0.001f);

	beam.update(svc, map, *this);
	auto bp = get_collider().get_center();
	bp.x += 4.f * directions.actual.as_float();
	bp.y -= 4.f;
	beam.get().set_barrel_point(bp);

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.beast.set(audio::Beast::hurt);
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();

	if (is_hostile() && !hostility_triggered() && !post_beam.running()) { request(BeamsproutState::charge); }
	if (directions.actual.lnr != directions.desired.lnr) { request(BeamsproutState::turn); }
	if (just_died()) { m_services->soundboard.flags.beamsprout.set(audio::Beamsprout::death); }

	state_function = state_function();
}

void Beamsprout::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
}

fsm::StateFunction Beamsprout::update_idle() {
	animation.label = "idle";
	p_state.actual = BeamsproutState::idle;
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(BeamsproutState::turn, get_params("turn"))) { return BEAMSPROUT_BIND(update_turn); }
	if (change_state(BeamsproutState::charge, get_params("charge"))) { return BEAMSPROUT_BIND(update_charge); }
	return BEAMSPROUT_BIND(update_idle);
};

fsm::StateFunction Beamsprout::update_charge() {
	animation.label = "charge";
	p_state.actual = BeamsproutState::charge;
	if (animation.just_started()) { m_services->soundboard.flags.beamsprout.set(audio::Beamsprout::charge); }
	if (animation.get_frame_count() > 7) {
		if (!has_flag_set(BeamsproutFlags::spit)) {
			beam.shoot(*m_services, *m_map);
			get_collider().physics.apply_force(directions.actual.as_float() * beam.get().get_recoil_force());
			m_root->variables.bob_physics.velocity.x = directions.actual.as_float() * beam.get().get_recoil_force().x * 2.f;
			set_flag(BeamsproutFlags::spit);
		}
	}
	if (animation.complete()) {
		post_beam.start();
		set_flag(BeamsproutFlags::spit, false);
		request(BeamsproutState::relax);
		if (change_state(BeamsproutState::relax, get_params("relax"))) { return BEAMSPROUT_BIND(update_relax); }
	}
	return BEAMSPROUT_BIND(update_charge);
};

fsm::StateFunction Beamsprout::update_shoot() {
	animation.label = "shoot";
	p_state.actual = BeamsproutState::shoot;
	if (m_services->ticker.every_x_ticks(static_cast<int>(fire_rate))) {
		beam.shoot(*m_services, *m_map);
		get_collider().physics.apply_force(directions.actual.as_float() * beam.get().get_recoil_force());
	}
	if (animation.complete()) {
		post_beam.start();
		request(BeamsproutState::relax);
		if (change_state(BeamsproutState::relax, get_params("relax"))) { return BEAMSPROUT_BIND(update_relax); }
	}
	return BEAMSPROUT_BIND(update_shoot);
}

fsm::StateFunction Beamsprout::update_relax() {
	animation.label = "relax";
	p_state.actual = BeamsproutState::relax;
	if (animation.complete()) {
		if (change_state(BeamsproutState::turn, get_params("turn"))) { return BEAMSPROUT_BIND(update_turn); }
		request(BeamsproutState::idle);
		if (change_state(BeamsproutState::idle, get_params("idle"))) { return BEAMSPROUT_BIND(update_idle); }
	}
	return BEAMSPROUT_BIND(update_relax);
}

fsm::StateFunction Beamsprout::update_turn() {
	p_state.actual = BeamsproutState::turn;
	if (animation.complete()) {
		m_root->variables.bob_physics.velocity.x = directions.actual.as_float() * 4.f;
		request_flip();
		request(BeamsproutState::relax);
		if (change_state(BeamsproutState::relax, get_params("relax"))) { return BEAMSPROUT_BIND(update_relax); }
	}
	return BEAMSPROUT_BIND(update_turn);
}

bool Beamsprout::change_state(BeamsproutState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

void Beamsprout::set_root(world::Map& map) {
	auto closest = std::numeric_limits<float>::max();
	for (auto& vine : map.get_entities<Vine>()) {
		for (auto& link : vine->get_chain().links) {
			auto distance = (get_collider().get_center() - link.get_bob()).length();
			if (distance < closest) {
				m_root = &link;
				closest = distance;
			}
		}
	}
}

} // namespace fornani::enemy
