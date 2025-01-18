#include "Tank.hpp"
#include "../../../level/Map.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Tank::Tank(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, "tank"), gun(svc, 0), m_services(&svc), m_map(&map) {
	animation.set_params(idle);
	gun.clip_cooldown_time = 360;
	gun.get().set_team(arms::Team::skycorps);
	collider.physics.maximum_velocity = {3.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	secondary_collider = shape::Collider({28.f, 28.f});
	directions.desired.lr = dir::LR::left;
	directions.actual.lr = dir::LR::left;
	directions.movement.lr = dir::LR::neutral;
}

void Tank::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	flags.state.set(StateFlags::vulnerable); // tank is always vulnerable
	gun.update(svc, map, *this);
	caution.avoid_ledges(map, collider, directions.actual, 1);

	// reset animation states to determine next animation state
	state = {};
	directions.desired.lr = (player.collider.get_center().x < collider.get_center().x) ? dir::LR::left : dir::LR::right;
	directions.movement.lr = collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;
	if (directions.actual.lr == dir::LR::right && visual.sprite.getScale() == sf::Vector2<float>{1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	if (directions.actual.lr == dir::LR::left && visual.sprite.getScale() == sf::Vector2<float>{-1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	Enemy::update(svc, map, player);
	secondary_collider.physics.position = collider.physics.position - sf::Vector2<float>{0.f, 14.f};
	secondary_collider.physics.position.x += directions.actual.lr == dir::LR::left ? 10.f : collider.dimensions.x - secondary_collider.dimensions.x - 10.f;
	secondary_collider.sync_components();
	
	player.collider.handle_collider_collision(secondary_collider);
	if (svc.ticker.every_x_ticks(20)) {
		if (svc.random.percent_chance(8) && !caution.danger()) { state = TankState::run; }
	}

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		if (m_services->random.percent_chance(50)) {
			m_services->soundboard.flags.tank.set(audio::Tank::hurt_1);
		} else {
			m_services->soundboard.flags.tank.set(audio::Tank::hurt_2);
		}
		hurt_effect.start(128);
		sound.hurt_sound_cooldown.start();
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();

	if (hostility_triggered()) { state = TankState::alert; }
	if (hostile() && !hostility_triggered()) {
		if (m_services->random.percent_chance(fire_chance) || caution.danger()) {
			state = TankState::shoot;
		} else {
			state = TankState::run;
		}
	} // player is already in hostile range

	if (just_died()) { m_services->soundboard.flags.tank.set(audio::Tank::death); }

	if (directions.actual.lr != directions.desired.lr) { state = TankState::turn; }

	state_function = state_function();
}

fsm::StateFunction Tank::update_idle() {
	animation.label = "idle";
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	if (change_state(TankState::alert, alert)) { return TANK_BIND(update_alert); }
	if (change_state(TankState::run, run)) { return TANK_BIND(update_run); }
	if (change_state(TankState::shoot, shoot)) { return TANK_BIND(update_shoot); }
	state = TankState::idle;
	return TANK_BIND(update_idle);
};
fsm::StateFunction Tank::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		visual.sprite.scale({-1.f, 1.f});
		directions.actual = directions.desired;
		state = TankState::idle;
		animation.set_params(idle, false);
		return TANK_BIND(update_idle);
	}
	state = TankState::turn;
	return TANK_BIND(update_turn);
};
fsm::StateFunction Tank::update_run() {
	animation.label = "run";
	auto facing = directions.actual.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({attributes.speed * facing, 0.f});
	if (caution.danger() || animation.complete()) {
		state = TankState::idle;
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	if (change_state(TankState::alert, alert)) { return TANK_BIND(update_alert); }
	state = TankState::run;
	return TANK_BIND(update_run);
}
fsm::StateFunction Tank::update_shoot() {
	animation.label = "shoot";
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	if (!gun.get().cooling_down()) {
		gun.cycle.update();
		if (directions.actual.lr == dir::LR::left) { gun.barrel_offset = gun.cycle.get_alternator() % 2 == 0 ? sf::Vector2<float>{-14.f, 10.f} : sf::Vector2<float>{26.f, 20.f}; }
		if (directions.actual.lr == dir::LR::right) { gun.barrel_offset = gun.cycle.get_alternator() % 2 == 1 ? sf::Vector2<float>{52.f, 10.f} : sf::Vector2<float>{14.f, 20.f}; }
		gun.shoot();
		m_map->spawn_projectile_at(*m_services, gun.get(), gun.barrel_point());
		m_services->soundboard.flags.weapon.set(audio::Weapon::skycorps_ar);
	}
	if (animation.complete() && animation.keyframe_over()) {
		gun.clip_cooldown.start(gun.clip_cooldown_time);
		state = TankState::idle;
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	state = TankState::shoot;
	return TANK_BIND(update_shoot);
}

fsm::StateFunction Tank::update_alert() { 
	animation.label = "alert";
	if (animation.just_started()) {
		if (m_services->random.percent_chance(50)) {
			m_services->soundboard.flags.tank.set(audio::Tank::alert_1);
		} else {
			m_services->soundboard.flags.tank.set(audio::Tank::alert_2);
		}
	}
	if (animation.complete()) {
		if (directions.actual.lr != directions.desired.lr) {
			state = TankState::turn;
			animation.set_params(turn);
			return TANK_BIND(update_turn);
		}
		state = TankState::shoot;
		animation.set_params(shoot);
		return TANK_BIND(update_shoot);
	}
	state = TankState::alert;
	return TANK_BIND(update_alert);
}

bool Tank::change_state(TankState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params, false);
		return true;
	}
	return false;
}

} // namespace enemy