
#include "fornani/entities/enemy/catalog/Meatsquash.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Meatsquash::Meatsquash(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "meatsquash"), m_services(&svc), m_map(&map) {
	p_animatable.set_animations({{"idle", {0, 6, 32, -1}}, {"chomp", {6, 12, 36, 0}}, {"open", {19, 4, 24, 0}}, {"swallow", {23, 13, 24, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_animatable.random_start();
	get_collider().physics.maximum_velocity = {8.f, 12.f};
	get_collider().physics.air_friction = {0.95f, 0.999f};
	get_collider().flags.general.set(shape::General::complex);

	directions.desired.lnr = LNR::right;
	directions.actual.lnr = LNR::right;
	directions.movement.lnr = LNR::neutral;
	get_collider().set_top_only();

	attacks.bite.sensor.bounds.setRadius(96.f);
	attacks.bite.hit.bounds.setRadius(86.f);
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
	attacks.bite.set_position(get_collider().get_center() + bite_offset);
	attacks.bite.update();

	if (attacks.bite.sensor.within_bounds(player.get_collider().bounding_box) && !player.is_dead()) { request(MeatsquashState::chomp); }

	auto has_no_collision = is_state(MeatsquashState::open) || (is_state(MeatsquashState::chomp) && p_animatable.animation.get_frame_count() > 5);
	has_no_collision ? flags.general.reset(GeneralFlags::player_collision) : flags.general.set(GeneralFlags::player_collision);
	auto active = p_animatable.animation.get_frame() == 12 && player.get_collider().get_center().y <= get_collider().physics.position.y;
	active ? attacks.bite.enable() : attacks.bite.disable();
	if (attacks.bite.kill_player(player, player::PlayerDeathType::swallowed)) { set_flag(MeatsquashFlags::swallowed_player); }

	if (has_flag_set(MeatsquashFlags::swallowed_player)) { request(MeatsquashState::swallow); }

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::hurt);
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();

	if (just_died()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::death); }

	state_function = state_function();
}

void Meatsquash::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (died()) { return; }
	Enemy::render(svc, win, cam);
	if (svc.greyblock_mode()) { attacks.bite.render(win, cam); }
}

fsm::StateFunction Meatsquash::update_idle() {
	p_animatable.animation.label = "idle";
	p_state.actual = MeatsquashState::idle;
	if (p_animatable.animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(MeatsquashState::chomp, get_params("chomp"))) { return MEATSQUASH_BIND(update_chomp); }
	return MEATSQUASH_BIND(update_idle);
};

fsm::StateFunction Meatsquash::update_chomp() {
	p_animatable.animation.label = "chomp";
	p_state.actual = MeatsquashState::chomp;
	if (p_animatable.animation.just_started()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::chomp); }
	if (p_animatable.animation.get_frame_count() == 5 && p_animatable.animation.keyframe_started()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::whip); }
	if (p_animatable.animation.complete()) {
		if (change_state(MeatsquashState::swallow, get_params("swallow"))) { return MEATSQUASH_BIND(update_swallow); }
		request(MeatsquashState::open);
		if (change_state(MeatsquashState::open, get_params("open"))) { return MEATSQUASH_BIND(update_open); }
	}
	return MEATSQUASH_BIND(update_chomp);
};

fsm::StateFunction Meatsquash::update_swallow() {
	p_animatable.animation.label = "swallow";
	p_state.actual = MeatsquashState::swallow;
	if (p_animatable.animation.get_frame_count() == 1 && p_animatable.animation.keyframe_started()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::swallow); }
	if (p_animatable.animation.complete()) {
		set_flag(MeatsquashFlags::swallowed_player, false);
		request(MeatsquashState::open);
		if (change_state(MeatsquashState::open, get_params("open"))) { return MEATSQUASH_BIND(update_open); }
	}
	return MEATSQUASH_BIND(update_swallow);
}

fsm::StateFunction Meatsquash::update_open() {
	p_animatable.animation.label = "open";
	p_state.actual = MeatsquashState::open;
	if (p_animatable.animation.just_started()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::open); }
	if (p_animatable.animation.complete()) {
		request(MeatsquashState::idle);
		if (change_state(MeatsquashState::idle, get_params("idle"))) { return MEATSQUASH_BIND(update_idle); }
	}
	return MEATSQUASH_BIND(update_open);
}

bool Meatsquash::change_state(MeatsquashState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
