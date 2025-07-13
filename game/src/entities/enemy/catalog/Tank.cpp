
#include "fornani/entities/enemy/catalog/Tank.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Tank::Tank(automa::ServiceProvider& svc, world::Map& map, int variant)
	: Enemy(svc, "tank"), m_variant{static_cast<TankVariant>(variant)}, m_weapon(svc, 6), m_services(&svc), m_map(&map), m_gun{svc.assets.get_texture("tank_gun"), 2.0f, 0.65f, {-12.f, 6.f}} {
	animation.set_params(idle);
	m_gun.set_magnitude(1.f);
	m_weapon.clip_cooldown_time = 360;
	m_weapon.get().set_team(arms::Team::skycorps);
	collider.physics.maximum_velocity = {3.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	secondary_collider = shape::Collider({28.f, 28.f});
	directions.desired.lnr = LNR::left;
	directions.actual.lnr = LNR::left;
	directions.movement.lnr = LNR::neutral;
	m_cooldowns.run.start();
	m_vertical_range.set_dimensions({90.f, 1024.f});
	m_lower_range.set_dimensions({200.f, 32.f});
	m_shoulders.set_dimensions({120.f, 32.f});
}

void Tank::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	flags.state.set(StateFlags::vulnerable); // tank is always vulnerable
	m_weapon.update(svc, map, *this);
	m_weapon.barrel_offset = sf::Vector2f{directions.actual.as_float() * 60.f, 8.f};
	m_caution.avoid_ledges(map, collider, directions.actual, 2);

	// reset animation states to determine next animation state
	directions.movement.lnr = collider.physics.velocity.x > 0.f ? LNR::right : LNR::left;
	secondary_collider.physics.position = collider.physics.position - sf::Vector2f{-26.f, 14.f};
	secondary_collider.sync_components();

	m_vertical_range.set_position(collider.bounding_box.get_position() - sf::Vector2f{(m_vertical_range.get_dimensions().x * 0.5f) - (collider.dimensions.x * 0.5f), (m_vertical_range.get_dimensions().y) - (collider.dimensions.y * 0.5f)});
	m_shoulders.set_position(collider.bounding_box.get_position() - sf::Vector2f{(m_shoulders.get_dimensions().x * 0.5f) - (collider.dimensions.x * 0.5f), m_shoulders.get_dimensions().y});
	m_lower_range.set_position(collider.bounding_box.get_position() - sf::Vector2f{(m_lower_range.get_dimensions().x * 0.5f) - (collider.dimensions.x * 0.5f), -(collider.dimensions.y - m_lower_range.get_dimensions().y)});

	auto has_clearance = !m_caution.detected_ceiling(map, collider, sf::Vector2f{0.f, 32.f});
	if (m_caution.detected_step(map, collider, directions.actual, sf::Vector2f{-16.f, 32.f}) && (collider.physics.is_moving_horizontally(0.01f) || is_mid_run()) && has_clearance) { request(TankState::jumpsquat); }

	player.collider.handle_collider_collision(secondary_collider);
	if (svc.ticker.every_x_ticks(20)) {
		if (util::random::percent_chance(8) && !m_caution.danger()) { request(TankState::run); }
	}

	m_sounds.hurt = util::random::percent_chance(50) ? audio::Tank::hurt_1 : audio::Tank::hurt_2;
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) { svc.soundboard.flags.tank.set(m_sounds.hurt); }
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
	}

	player.on_crush(map);

	if (is_alert() && has_clearance) { request(TankState::jumpsquat); }
	if (is_hostile() && has_been_alerted()) { request(TankState::shoot_horizontal); }
	if ((is_hostile() || is_alert()) && !has_been_alerted()) { request(TankState::alert); }

	if (player.collider.bounding_box.overlaps(m_vertical_range) && has_been_alerted()) { request(TankState::shoot_vertical); }
	if (player.collider.bounding_box.overlaps(m_shoulders)) {
		request(TankState::jumpsquat);
		m_flags.set(TankFlags::shorthop);
	}
	if (player.collider.bounding_box.overlaps(m_lower_range) && has_been_alerted()) { request(TankState::jumpsquat); }

	if (just_died()) { m_services->soundboard.flags.tank.set(audio::Tank::death); }

	if (hostility_triggered() || alertness_triggered()) { m_cooldowns.alerted.start(); }
	// try to go find a seat
	if (!has_been_alerted()) {
		if (seek_home(map)) { m_variant == TankVariant::typist ? request(TankState::type) : request(TankState::sleep); }
		if (flags.state.test(StateFlags::advance)) { request(TankState::run); }
	} else {
		face_player(player);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(TankState::turn); }

	state_function = state_function();

	auto gun_offset = m_state.actual == TankState::pocket ? sf::Vector2f{directions.actual.as_float() * -30.f, 12.f} : sf::Vector2f{0.f, 4.f};
	m_gun.update(svc, map, player, directions.actual, Drawable::get_scale(), collider.get_center() + gun_offset);

	hurt_effect.update();
	m_cooldowns.post_jump.update();
	m_cooldowns.alerted.update();
}

void Tank::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
	if (m_state.actual != TankState::shoot_vertical) { m_gun.sprite->setRotation(sf::degrees(0)); }
	if (m_flags.test(TankFlags::show_weapon)) { m_gun.render(svc, win, cam); }
	if (svc.greyblock_mode()) {
		m_vertical_range.render(win, cam);
		m_caution.debug_render(win, cam);
		m_shoulders.render(win, cam, sf::Color{230, 0, 20, 80});
		m_lower_range.render(win, cam, sf::Color{130, 0, 230, 80});
	}
}

fsm::StateFunction Tank::update_idle() {
	animation.label = "idle";
	m_state.actual = TankState::idle;
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	if (is_hostile() && !has_been_alerted()) { request(TankState::alert); }
	if (m_cooldowns.alerted.is_complete() && m_flags.test(TankFlags::show_weapon)) { request(TankState::pocket); }
	if (change_state(TankState::run, run)) { return TANK_BIND(update_run); }
	if (change_state(TankState::pocket, pocket)) { return TANK_BIND(update_pocket); }
	if (change_state(TankState::alert, alert)) { return TANK_BIND(update_alert); }
	if (change_state(TankState::jumpsquat, jumpsquat)) { return TANK_BIND(update_jumpsquat); }
	if (has_been_alerted()) {
		if (change_state(TankState::shoot_horizontal, shoot_horizontal)) { return TANK_BIND(update_shoot_horizontal); }
		if (change_state(TankState::shoot_vertical, shoot_vertical)) { return TANK_BIND(update_shoot_vertical); }
	}
	if (change_state(TankState::type, type)) { return TANK_BIND(update_type); }
	if (change_state(TankState::sleep, sleep)) { return TANK_BIND(update_sleep); }
	return TANK_BIND(update_idle);
}

fsm::StateFunction Tank::update_turn() {
	animation.label = "turn";
	m_state.actual = TankState::turn;
	directions.desired.lock();
	if (animation.complete()) {
		request_flip();
		if (change_state(TankState::type, type)) { return TANK_BIND(update_type); }
		if (change_state(TankState::sleep, sleep)) { return TANK_BIND(update_sleep); }
		request(TankState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	return TANK_BIND(update_turn);
}

fsm::StateFunction Tank::update_type() {
	animation.label = "type";
	m_state.actual = TankState::type;
	if (m_services->ticker.every_second()) {
		if (util::random::percent_chance(10)) { request(TankState::drink); }
		if (change_state(TankState::drink, drink)) { return TANK_BIND(update_drink); }
	}
	if (hostility_triggered()) { request(TankState::alert); }
	if (is_hurt()) { request(TankState::alert); }
	if (change_state(TankState::alert, alert)) { return TANK_BIND(update_alert); }
	if (change_state(TankState::run, run)) { return TANK_BIND(update_run); }
	if (change_state(TankState::idle, idle)) { return TANK_BIND(update_idle); }
	return TANK_BIND(update_type);
}

fsm::StateFunction Tank::update_run() {
	animation.label = "run";
	m_state.actual = TankState::run;
	collider.physics.apply_force({attributes.speed * directions.actual.as_float(), 0.f});
	m_cooldowns.run.update();
	if (change_state(TankState::turn, turn)) {
		m_cooldowns.run.start();
		return TANK_BIND(update_turn);
	}
	if (change_state(TankState::jumpsquat, jumpsquat)) {
		m_cooldowns.run.start();
		return TANK_BIND(update_jumpsquat);
	}
	if (change_state(TankState::alert, alert)) {
		m_cooldowns.run.start();
		return TANK_BIND(update_alert);
	}
	if (change_state(TankState::type, type)) {
		m_cooldowns.run.start();
		return TANK_BIND(update_type);
	}
	if (change_state(TankState::sleep, sleep)) {
		m_cooldowns.run.start();
		return TANK_BIND(update_sleep);
	}
	if (m_caution.danger() || animation.complete()) {
		request(TankState::idle);
		m_cooldowns.run.start();
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	if (change_state(TankState::shoot_horizontal, shoot_horizontal)) {
		m_cooldowns.run.start();
		return TANK_BIND(update_shoot_horizontal);
	}
	if (change_state(TankState::shoot_vertical, shoot_vertical)) {
		m_cooldowns.run.start();
		return TANK_BIND(update_shoot_vertical);
	}
	return TANK_BIND(update_run);
}

fsm::StateFunction Tank::update_shoot_horizontal() {
	animation.label = "shoot";
	m_state.actual = TankState::shoot_horizontal;
	m_flags.set(TankFlags::show_weapon);
	if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
	if (!m_weapon.get().cooling_down() && animation.get_frame_count() == 2) {
		m_weapon.shoot(*m_services, *m_map);
		m_gun.set_position(m_gun.get_position() + directions.actual.get_vector() * -18.f);
	}
	if (animation.complete() && animation.keyframe_over()) {
		m_weapon.clip_cooldown.start(m_weapon.clip_cooldown_time);
		request(TankState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	return TANK_BIND(update_shoot_horizontal);
}

fsm::StateFunction Tank::update_jumpsquat() {
	animation.label = "jumpsquat";
	m_state.actual = TankState::jumpsquat;
	m_flags.set(TankFlags::show_weapon);
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
	m_state.actual = TankState::jump;
	if (m_flags.test(TankFlags::shorthop)) {
		collider.physics.apply_force({-attributes.speed * directions.actual.as_float(), 0.f});
		if (impulse.running()) { collider.physics.apply_force({0, -40.f}); }
	} else {
		collider.physics.apply_force({attributes.speed * directions.actual.as_float(), 0.f});
		if (impulse.running()) { collider.physics.apply_force({0, -100.f}); }
	}
	if (animation.complete() && collider.grounded()) {
		m_flags.reset(TankFlags::shorthop);
		request(TankState::land);
		m_services->soundboard.flags.world.set(audio::World::thud);
		m_services->camera_controller.shake(10, 0.3f, 200, 20);
		m_cooldowns.post_jump.start();
		if (change_state(TankState::land, land)) { return TANK_BIND(update_land); }
	}
	return TANK_BIND(update_jump);
}

fsm::StateFunction Tank::update_land() {
	animation.label = "land";
	m_state.actual = TankState::land;
	if (animation.complete()) {
		if (change_state(TankState::shoot_horizontal, shoot_horizontal)) { return TANK_BIND(update_shoot_horizontal); }
		if (change_state(TankState::shoot_vertical, shoot_vertical)) { return TANK_BIND(update_shoot_vertical); }
		if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
		request(TankState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	return TANK_BIND(update_land);
}

fsm::StateFunction Tank::update_shoot_vertical() {
	animation.label = "shoot_vertical";
	m_state.actual = TankState::shoot_vertical;
	m_flags.set(TankFlags::show_weapon);
	m_weapon.barrel_offset = sf::Vector2f{0.f, -40.f};
	directions.actual.und = UND::up;
	auto degrees = directions.actual.left() ? 90 : -90;
	m_gun.sprite->setRotation(sf::degrees(degrees));

	auto slide = directions.actual.left() ? 8.f : -8.f;
	m_gun.move(sf::Vector2f{slide, -8.f});
	m_weapon.update(*m_services, *m_map, *this);
	if (!m_weapon.get().cooling_down() && animation.get_frame_count() == 0) { m_weapon.shoot(*m_services, *m_map); }
	if (animation.complete() && animation.keyframe_over()) {
		m_gun.sprite->setRotation(sf::degrees(0));
		directions.actual.neutralize_und();
		m_weapon.clip_cooldown.start(m_weapon.clip_cooldown_time);
		if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
		if (change_state(TankState::run, run)) { return TANK_BIND(update_run); }
		if (change_state(TankState::shoot_vertical, shoot_vertical)) { return TANK_BIND(update_shoot_vertical); }
		request(TankState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	return TANK_BIND(update_shoot_vertical);
}

fsm::StateFunction Tank::update_alert() {
	animation.label = "alert";
	m_state.actual = TankState::alert;
	if (animation.just_started()) {
		m_cooldowns.alerted.start();
		if (util::random::percent_chance(50)) {
			m_services->soundboard.flags.tank.set(audio::Tank::alert_1);
		} else {
			m_services->soundboard.flags.tank.set(audio::Tank::alert_2);
		}
	}
	if (animation.complete()) {
		request(TankState::pocket);
		if (change_state(TankState::pocket, pocket)) { return TANK_BIND(update_pocket); }
	}
	return TANK_BIND(update_alert);
}

fsm::StateFunction Tank::update_pocket() {
	animation.label = "pocket";
	m_state.actual = TankState::pocket;
	if (animation.complete()) {
		m_flags.toggle(TankFlags::show_weapon);
		if (change_state(TankState::turn, turn)) { return TANK_BIND(update_turn); }
		if (change_state(TankState::shoot_vertical, shoot_vertical)) { return TANK_BIND(update_shoot_vertical); }
		has_been_alerted() ? request(TankState::shoot_horizontal) : request(TankState::idle);
		if (change_state(TankState::shoot_horizontal, shoot_horizontal)) { return TANK_BIND(update_shoot_horizontal); }
		if (change_state(TankState::idle, idle)) { return TANK_BIND(update_idle); }
	}
	return TANK_BIND(update_pocket);
}

fsm::StateFunction Tank::update_sleep() {
	animation.label = "sleep";
	m_state.actual = TankState::sleep;
	if (hostility_triggered()) { request(TankState::alert); }
	if (is_hurt()) { request(TankState::alert); }
	if (ccm::abs(collider.physics.actual_velocity().x) > 0.01f) { request(TankState::idle); }
	if (change_state(TankState::alert, alert)) { return TANK_BIND(update_alert); }
	if (change_state(TankState::idle, idle)) { return TANK_BIND(update_idle); }
	return TANK_BIND(update_sleep);
}

fsm::StateFunction Tank::update_drink() {
	animation.label = "drink";
	m_state.actual = TankState::drink;
	if (animation.complete()) {
		request(TankState::type);
		if (is_hurt()) { request(TankState::alert); }
		if (change_state(TankState::type, type)) { return TANK_BIND(update_type); }
		if (change_state(TankState::alert, alert)) { return TANK_BIND(update_alert); }
	}
	return TANK_BIND(update_drink);
}

bool Tank::change_state(TankState next, anim::Parameters params) {
	if (m_state.desired == next) {
		animation.set_params(params, false);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
