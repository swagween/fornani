
#include <fornani/entities/enemy/catalog/Antibody.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Antibody::Antibody(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "antibody"), m_services{&svc}, m_post_stun{400}, m_variant{static_cast<AntibodyVariant>(variant)} {
	p_animations = {{"idle", {0, 8, 20, -1}}, {"stun", {8, 7, 32, 0}}};
	animation.set_params(get_params("idle"));
	p_state.actual = AntibodyState::idle;

	get_collider().set_flag(shape::ColliderFlags::simple);
	variant == 0 ? get_collider().physics.set_friction_componentwise({0.98f, 0.98f}) : get_collider().physics.set_friction_componentwise({0.985f, 0.985f});
	flags.state.set(StateFlags::vulnerable);

	flags.general.set(GeneralFlags::custom_channels);
	m_custom_channel = static_cast<EnemyChannel>(variant);
}

void Antibody::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }
	m_post_stun.update();

	auto lower_bound = m_variant == AntibodyVariant::igg ? 8.f : 6.f;
	auto upper_bound = m_variant == AntibodyVariant::igg ? 48.f : 24.f;
	set_framerate(static_cast<int>(std::lerp(lower_bound, upper_bound, std::clamp((player.get_collider().get_center() - get_collider().get_center()).length() / 260.f, 0.f, 1.f))));

	if (player.hurtbox.overlaps(get_collider().bounding_box) && player.can_be_stunned() && !m_post_stun.running()) {
		m_variant == AntibodyVariant::igg ? player.stun(0.8f) : player.hurt_and_stun();
		request(AntibodyState::stun);
	}
	if ((is_alert() && !m_post_stun.running()) || !map.within_bounds(get_collider().get_center())) {
		auto force = m_variant == AntibodyVariant::igg ? 0.02f : 0.025f;
		m_steering.thrust_seek(get_collider().physics, player.get_collider().get_center(), ThrustParameters{force, .218f, .999, 40.f});
	} else {
		m_steering.smooth_random_walk(get_collider().physics, 0.005f, 64.f);
	}

	state_function = state_function();
}

void Antibody::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Enemy::render(svc, win, cam); }

fsm::StateFunction Antibody::update_idle() {
	p_state.actual = AntibodyState::idle;
	if (change_state(AntibodyState::stun, get_params("stun"))) { return ANTIBODY_BIND(update_stun); }
	return ANTIBODY_BIND(update_idle);
}

fsm::StateFunction Antibody::update_stun() {
	p_state.actual = AntibodyState::stun;
	if (animation.get_frame_count() < 2) { shake(); }
	if (animation.complete()) {
		m_post_stun.start();
		request(AntibodyState::idle);
		if (change_state(AntibodyState::idle, get_params("idle"))) { return ANTIBODY_BIND(update_idle); }
	}
	return ANTIBODY_BIND(update_stun);
}

bool Antibody::change_state(AntibodyState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
