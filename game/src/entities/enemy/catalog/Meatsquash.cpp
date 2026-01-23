
#include "fornani/entities/enemy/catalog/Meatsquash.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Meatsquash::Meatsquash(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "meatsquash"), m_services(&svc), m_map(&map) {
	m_params = {{"idle", {0, 6, 36, -1}}, {"chomp", {6, 12, 36, 0}}, {"open", {19, 4, 24, 0}}, {"swallow", {23, 13, 24, 0}}};
	animation.set_params(get_params("idle"));
	get_collider().physics.maximum_velocity = {8.f, 12.f};
	get_collider().physics.air_friction = {0.95f, 0.999f};
	get_collider().flags.general.set(shape::General::complex);

	directions.desired.lnr = LNR::right;
	directions.actual.lnr = LNR::right;
	directions.movement.lnr = LNR::neutral;
	get_collider().set_top_only();

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
	attacks.bite.set_position(get_collider().get_center() + bite_offset);
	attacks.bite.update();
	attacks.bite.handle_player(player);

	if (is_hostile() && !hostility_triggered() && !player.is_dead()) { request(MeatsquashState::chomp); }

	auto has_no_collision = is_state(MeatsquashState::open) || (is_state(MeatsquashState::chomp) && animation.get_frame_count() > 5);
	has_no_collision ? flags.general.reset(GeneralFlags::player_collision) : flags.general.set(GeneralFlags::player_collision);
	animation.get_frame() == 12 ? attacks.bite.hit.activate() : attacks.bite.hit.deactivate();
	if (attacks.bite.sensor.active() && attacks.bite.hit.active() && !(player.get_collider().get_center().y > get_collider().physics.position.y)) {
		player.set_death_type(player::PlayerDeathType::swallowed);
		set_flag(MeatsquashFlags::swallowed_player);
		player.hurt(24.f);
	}

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
	animation.label = "idle";
	p_state.actual = MeatsquashState::idle;
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(MeatsquashState::chomp, get_params("chomp"))) { return MEATSQUASH_BIND(update_chomp); }
	return MEATSQUASH_BIND(update_idle);
};

fsm::StateFunction Meatsquash::update_chomp() {
	animation.label = "chomp";
	p_state.actual = MeatsquashState::chomp;
	if (animation.just_started()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::chomp); }
	if (animation.get_frame_count() == 5 && animation.keyframe_started()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::whip); }
	if (animation.complete()) {
		if (change_state(MeatsquashState::swallow, get_params("swallow"))) { return MEATSQUASH_BIND(update_swallow); }
		request(MeatsquashState::open);
		if (change_state(MeatsquashState::open, get_params("open"))) { return MEATSQUASH_BIND(update_open); }
	}
	return MEATSQUASH_BIND(update_chomp);
};

fsm::StateFunction Meatsquash::update_swallow() {
	animation.label = "swallow";
	p_state.actual = MeatsquashState::swallow;
	if (animation.get_frame_count() == 1 && animation.keyframe_started()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::swallow); }
	if (animation.complete()) {
		set_flag(MeatsquashFlags::swallowed_player, false);
		request(MeatsquashState::open);
		if (change_state(MeatsquashState::open, get_params("open"))) { return MEATSQUASH_BIND(update_open); }
	}
	return MEATSQUASH_BIND(update_swallow);
}

fsm::StateFunction Meatsquash::update_open() {
	animation.label = "open";
	p_state.actual = MeatsquashState::open;
	if (animation.just_started()) { m_services->soundboard.flags.meatsquash.set(audio::Meatsquash::open); }
	if (animation.complete()) {
		request(MeatsquashState::idle);
		if (change_state(MeatsquashState::idle, get_params("idle"))) { return MEATSQUASH_BIND(update_idle); }
	}
	return MEATSQUASH_BIND(update_open);
}

bool Meatsquash::change_state(MeatsquashState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
