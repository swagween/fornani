
#include <fornani/entities/enemy/catalog/Hurtle.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Hurtle::Hurtle(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "hurtle"), Animatable{svc, "enemy_hurtle", {44, 44}}, m_services(&svc), m_map(&map), m_turn{200} {
	p_animations = {{"run", {0, 4, 30, -1}}, {"sleep", {4, 1, 80, 0}}, {"turn", {5, 1, 30, 0}}};
	animation.set_params(get_params("run"));
	get_collider().physics.set_friction_componentwise({0.98f, 0.999f});
	attributes.team = arms::Team::guardian;
	get_secondary_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	get_secondary_collider().set_attribute(shape::ColliderAttributes::no_collision);
	get_secondary_collider().set_exclusion_target(shape::ColliderTrait::player);
	get_secondary_collider().set_resolution_exclusion_target(shape::ColliderTrait::player);
	get_secondary_collider().set_dimensions({60.f, 32.f});
	flags.general.set(GeneralFlags::invincible_secondary);
}

void Hurtle::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { m_services->soundboard.play_sound("hellion_death", get_collider().get_center()); }
	Enemy::update(svc, map, player);
	if (died()) { return; }

	m_turn.update();

	is_state(HurtleState::sleep) ? flags.state.reset(StateFlags::vulnerable) : flags.state.set(StateFlags::vulnerable);

	hurt_effect.update();
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) { svc.soundboard.play_sound("hit_squeak"); }
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
	}

	if (secondary_collider) {
		get_secondary_collider().physics.position = get_collider().physics.position + sf::Vector2f{-6.f, -8.f};
		get_secondary_collider().sync_components();
		if (player.hurtbox.overlaps(get_secondary_collider().bounding_box)) { player.hurt(); }
	}

	if ((get_collider().has_left_wallslide_collision() || get_collider().has_right_wallslide_collision()) && !m_turn.running()) {
		directions.desired = get_collider().has_left_wallslide_collision() ? Direction{LR::right} : Direction{LR::left};
		request(HurtleState::turn);
	}

	// caution
	if (m_caution.is_ledge_detected(map, get_collider(), directions.actual, 2) && !m_turn.running()) {
		directions.desired = m_caution.direction;
		request(HurtleState::turn);
	}
	if (m_caution.is_projectile_detected(map, physical.alert_range, arms::Team::beast)) { request(HurtleState::sleep); }

	state_function = state_function();
}

void Hurtle::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
	if (svc.greyblock_mode()) { m_caution.debug_render(win, cam); }
}

fsm::StateFunction Hurtle::update_run() {
	p_state.actual = HurtleState::run;
	auto spd = animation.get_frame_count() % 2 == 0 ? attributes.speed : attributes.speed * 0.6f;
	get_collider().physics.velocity.x = directions.actual.as_float() * spd;
	if (change_state(HurtleState::sleep, get_params("sleep"))) { return HURTLE_BIND(update_sleep); }
	if (change_state(HurtleState::turn, get_params("turn"))) { return HURTLE_BIND(update_turn); }
	return HURTLE_BIND(update_run);
}

fsm::StateFunction Hurtle::update_sleep() {
	p_state.actual = HurtleState::sleep;
	if (animation.complete()) {
		if (m_caution.is_projectile_detected(*m_map, physical.alert_range, arms::Team::beast)) {
			request(HurtleState::sleep);
			if (change_state(HurtleState::sleep, get_params("sleep"))) { return HURTLE_BIND(update_sleep); }
		}
		request(HurtleState::run);
		if (change_state(HurtleState::run, get_params("run"))) { return HURTLE_BIND(update_run); }
	}
	return HURTLE_BIND(update_sleep);
}

fsm::StateFunction Hurtle::update_turn() {
	p_state.actual = HurtleState::turn;
	directions.desired.lock();
	if (animation.complete()) {
		m_turn.start();
		request_flip();
		request(HurtleState::run);
		if (change_state(HurtleState::run, get_params("run"))) { return HURTLE_BIND(update_run); }
	}
	return HURTLE_BIND(update_turn);
}

bool Hurtle::change_state(HurtleState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
