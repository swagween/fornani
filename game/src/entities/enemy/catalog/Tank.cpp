
#include "fornani/entities/enemy/catalog/Tank.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Tank::Tank(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, "tank"), m_weapon(svc, 0), m_services(&svc), m_map(&map), m_gun{svc.assets.get_texture("tank_gun"), 2.0f, 0.85f, {-12.f, 6.f}}

{
	animation.set_params(idle);
	m_weapon.clip_cooldown_time = 360;
	m_weapon.get().set_team(arms::Team::skycorps);
	collider.physics.maximum_velocity = {3.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	secondary_collider = shape::Collider({28.f, 28.f});
	directions.desired.lnr = LNR::left;
	directions.actual.lnr = LNR::left;
	directions.movement.lnr = LNR::neutral;
}

void Tank::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	flags.state.set(StateFlags::vulnerable); // tank is always vulnerable
	m_weapon.update(svc, map, *this);
	caution.avoid_ledges(map, collider, directions.actual, 1);

	// reset animation states to determine next animation state
	face_player(player);
	directions.movement.lnr = collider.physics.velocity.x > 0.f ? LNR::right : LNR::left;
	secondary_collider.physics.position = collider.physics.position - sf::Vector2<float>{-26.f, 14.f};
	secondary_collider.sync_components();

	player.collider.handle_collider_collision(secondary_collider);
	if (svc.ticker.every_x_ticks(20)) {
		if (util::random::percent_chance(8) && !caution.danger()) { request(TankState::run); }
	}

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		if (util::random::percent_chance(50)) {
			m_services->soundboard.flags.tank.set(audio::Tank::hurt_1);
		} else {
			m_services->soundboard.flags.tank.set(audio::Tank::hurt_2);
		}
		sound.hurt_sound_cooldown.start();
	}

	hurt_effect.update();
	player.on_crush(map);

	if (hostility_triggered()) { request(TankState::alert); }
	if (hostile() && !hostility_triggered()) {
		if (util::random::percent_chance(fire_chance) || caution.danger()) {
			request(TankState::shoot_horizontal);
		} else {
			request(TankState::run);
		}
	} // player is already in hostile range

	if (just_died()) { m_services->soundboard.flags.tank.set(audio::Tank::death); }

	if (is_hurt()) { request(TankState::jumpsquat); }

	if (directions.actual.lnr != directions.desired.lnr) { request(TankState::turn); }

	state_function = state_function();
}

void Tank::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { Enemy::render(svc, win, cam); }

fsm::StateFunction Tank::update_idle() {
	animation.label = "idle";
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	if (change_state(TankState::alert, alert)) { return TANK_BIND(update_alert); }
	if (change_state(TankState::jumpsquat, jumpsquat)) { return TANK_BIND(update_jumpsquat); }
	if (change_state(TankState::run, run)) { return TANK_BIND(update_run); }
	if (change_state(TankState::shoot_horizontal, shoot_horizontal)) { return TANK_BIND(update_shoot_horizontal); }
	request(TankState::idle);
	return TANK_BIND(update_idle);
}

fsm::StateFunction Tank::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		request_flip();
		request(TankState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	return TANK_BIND(update_turn);
}

fsm::StateFunction Tank::update_type() {
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	return TANK_BIND(update_type);
}

fsm::StateFunction Tank::update_run() {
	animation.label = "run";
	collider.physics.apply_force({attributes.speed * directions.actual.as_float(), 0.f});
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	if (caution.danger() || animation.complete()) {
		request(TankState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	if (change_state(TankState::alert, alert)) { return TANK_BIND(update_alert); }
	request(TankState::run);
	return TANK_BIND(update_run);
}

fsm::StateFunction Tank::update_shoot_horizontal() {
	animation.label = "shoot";
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	if (!m_weapon.get().cooling_down()) {
		m_weapon.shoot();
		m_map->spawn_projectile_at(*m_services, m_weapon.get(), m_weapon.barrel_point());
		m_services->soundboard.flags.weapon.set(audio::Weapon::skycorps_ar);
	}
	if (animation.complete() && animation.keyframe_over()) {
		m_weapon.clip_cooldown.start(m_weapon.clip_cooldown_time);
		request(TankState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	request(TankState::shoot_horizontal);
	return TANK_BIND(update_shoot_horizontal);
}

fsm::StateFunction Tank::update_jumpsquat() {
	animation.label = "jumpsquat";
	if (animation.complete()) {
		request(TankState::jump);
		if (change_state(TankState::jump, jump)) {
			impulse.start(3);
			m_services->soundboard.flags.enemy.set(audio::Enemy::jump_low);
			return TANK_BIND(update_jump);
		}
	}
	return TANK_BIND(update_jumpsquat);
}

fsm::StateFunction Tank::update_jump() {
	animation.label = "jump";
	if (impulse.running()) { collider.physics.apply_force({0, -100.f}); }
	if (animation.complete() && collider.grounded()) {
		request(TankState::land);
		m_services->soundboard.flags.world.set(audio::World::thud);
		m_services->camera_controller.shake(10, 0.3f, 200, 20);
		if (change_state(TankState::land, land)) { return TANK_BIND(update_land); }
	}
	return TANK_BIND(update_jump);
}

fsm::StateFunction Tank::update_land() {
	animation.label = "land";
	if (animation.complete()) {
		request(TankState::land);
		if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
		request(TankState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	return TANK_BIND(update_land);
}

fsm::StateFunction Tank::update_shoot_vertical() {
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	return TANK_BIND(update_shoot_vertical);
}

fsm::StateFunction Tank::update_alert() {
	animation.label = "alert";
	if (animation.just_started()) {
		if (util::random::percent_chance(50)) {
			m_services->soundboard.flags.tank.set(audio::Tank::alert_1);
		} else {
			m_services->soundboard.flags.tank.set(audio::Tank::alert_2);
		}
	}
	if (animation.complete()) {
		if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
		request(TankState::shoot_horizontal);
		animation.set_params(shoot_horizontal);
		return TANK_BIND(update_shoot_horizontal);
	}
	request(TankState::alert);
	return TANK_BIND(update_alert);
}

bool Tank::change_state(TankState next, anim::Parameters params) {
	if (m_state.desired == next) {
		animation.set_params(params, false);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
