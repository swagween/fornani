#include "Thug.hpp"
#include "../../../level/Map.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Thug::Thug(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, "thug"), m_services(&svc), m_map(&map) {
	animation.set_params(idle);
	collider.physics.maximum_velocity = {8.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	secondary_collider = shape::Collider({28.f, 28.f});
	directions.desired.lr = dir::LR::left;
	directions.actual.lr = dir::LR::left;
	directions.movement.lr = dir::LR::neutral;
	attacks.punch.sensor.bounds.setRadius(10);
	attacks.punch.sensor.drawable.setFillColor(svc.styles.colors.blue);
	attacks.punch.hit.bounds.setRadius(28);
	attacks.punch.origin = {-10.f, -26.f};

	attacks.rush.sensor.bounds.setRadius(40);
	attacks.rush.hit.bounds.setRadius(40);
	attacks.rush.origin = {20.f, 16.f};
	attacks.rush.hit_offset = {0.f, 0.f};
}

void Thug::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}
	if (directions.actual.lr == dir::LR::left) {
		attacks.punch.set_position(Enemy::collider.physics.position);
		attacks.rush.set_position(Enemy::collider.physics.position);
		attacks.punch.origin.x = -10.f;
		attacks.rush.origin.x = 20.f;
		attacks.rush.hit_offset.x = 0.f;
	} else {
		sf::Vector2<float> dir_offset{Enemy::collider.bounding_box.dimensions.x, 0.f};
		attacks.punch.set_position(Enemy::collider.physics.position + dir_offset);
		attacks.rush.set_position(Enemy::collider.physics.position + dir_offset);
		attacks.punch.origin.x = 10.f;
		attacks.rush.origin.x = -20.f;
		attacks.rush.hit_offset.x = 0.f;
	}

	cooldowns.rush_hit.update();

	attacks.punch.update();
	attacks.rush.update();
	attacks.punch.handle_player(player);
	attacks.rush.handle_player(player);
	if (state == ThugState::rush && attacks.rush.sensor.active() && !cooldowns.rush_hit.running()) {
		auto sign = directions.actual.lr == dir::LR::left ? -1.f : 1.f;
		if ((sign == -1.f && player_behind(player)) || (sign == 1.f && !player_behind(player))) {
			player.hurt(1);
			player.accumulated_forces.push_back({sign * 2.f, -2.f});
			attacks.rush.sensor.deactivate();
			cooldowns.rush_hit.start();
		}
	}

	flags.state.set(StateFlags::vulnerable); // thug is always vulnerable
	caution.avoid_ledges(map, collider, directions.actual, 1);

	// reset animation states to determine next animation state
	state = {};
	directions.desired.lr = (player.collider.get_center().x < collider.get_center().x) ? dir::LR::left : dir::LR::right;
	directions.movement.lr = collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;
	if (directions.actual.lr == dir::LR::right && sprite.getScale() == sf::Vector2<float>{1.f, 1.f}) { sprite.scale({-1.f, 1.f}); }
	if (directions.actual.lr == dir::LR::left && sprite.getScale() == sf::Vector2<float>{-1.f, 1.f}) { sprite.scale({-1.f, 1.f}); }
	Enemy::update(svc, map, player);
	secondary_collider.physics.position = collider.physics.position - sf::Vector2<float>{0.f, 14.f};
	secondary_collider.physics.position.x += directions.actual.lr == dir::LR::left ? 10.f : collider.dimensions.x - secondary_collider.dimensions.x - 10.f;
	secondary_collider.sync_components();
	player.collider.handle_collider_collision(secondary_collider.bounding_box);

	if (svc.ticker.every_x_ticks(200)) {
		if (svc.random.percent_chance(20) && !caution.danger()) { state = ThugState::run; }
	}

	if(flags.state.test(StateFlags::hurt)) {
		if (m_services->random.percent_chance(50)) {
			m_services->soundboard.flags.thug.set(audio::Thug::hurt_1);
		} else {
			m_services->soundboard.flags.thug.set(audio::Thug::hurt_2);
		}
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();
	if (hurt_effect.running()) {
		if ((hurt_effect.get_cooldown() / 32) % 2 == 0) {
			sprite.setColor(svc.styles.colors.red);
		} else {
			sprite.setColor(svc.styles.colors.periwinkle);
		}
	} else {
		sprite.setColor(svc.styles.colors.white);
	}

	if (hostility_triggered()) { state = ThugState::alert; }
	if (hostile() && !hostility_triggered()) {
		state = ThugState::run;
	} // player is already in hostile range

	if (just_died()) { m_services->soundboard.flags.thug.set(audio::Thug::death); }

	if (directions.actual.lr != directions.desired.lr) { state = ThugState::turn; }

	state_function = state_function();
}

void Thug::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (!svc.greyblock_mode()) {
	} else {
		if (state == ThugState::punch) { attacks.punch.render(win, cam); }
		if (caution.danger()) { attacks.rush.render(win, cam); }
		auto& probe = directions.actual.lr == dir::LR::left ? caution.testers.left : caution.testers.right;
		sf::CircleShape pt{};
		pt.setPointCount(8);
		pt.setRadius(4);
		for (auto& point : probe) {
			point.second ? pt.setFillColor(sf::Color::Green) : pt.setFillColor(sf::Color::Red);
			pt.setPosition(point.first - cam);
			win.draw(pt);
			if (point.second) { break; }
		}
	}
}

fsm::StateFunction Thug::update_idle() {
	animation.label = "idle";
	if (change_state(ThugState::turn, turn)) { return THUG_BIND(update_turn); }
	if (change_state(ThugState::alert, alert)) { return THUG_BIND(update_alert); }
	if (change_state(ThugState::run, run)) { return THUG_BIND(update_run); }
	if (change_state(ThugState::jump, jump)) { return THUG_BIND(update_jump); }
	state = ThugState::idle;
	return THUG_BIND(update_idle);
};
fsm::StateFunction Thug::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		Enemy::sprite.scale({-1.f, 1.f});
		directions.actual = directions.desired;
		state = ThugState::idle;
		animation.set_params(idle, false);
		return THUG_BIND(update_idle);
	}
	state = ThugState::turn;
	return THUG_BIND(update_turn);
};
fsm::StateFunction Thug::update_run() {
	animation.label = "run";
	auto facing = directions.actual.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({attributes.speed * facing, 0.f});
	if (caution.danger() || animation.complete()) {
		state = ThugState::idle;
		animation.set_params(idle);
		return THUG_BIND(update_idle);
	}
	if (change_state(ThugState::turn, turn)) { return THUG_BIND(update_turn); }
	if (change_state(ThugState::alert, alert)) { return THUG_BIND(update_alert); }
	state = ThugState::run;
	return THUG_BIND(update_run);
}
fsm::StateFunction Thug::update_jump() {
	animation.label = "jump";
	if (change_state(ThugState::turn, turn)) { return THUG_BIND(update_turn); }
	if (animation.just_started()) { cooldowns.jump.start(); }
	cooldowns.jump.update();
	if (cooldowns.jump.running()) { collider.physics.apply_force({-2.f, -8.f}); }
	if (animation.complete()) {
		state = ThugState::idle;
		animation.set_params(idle);
		return THUG_BIND(update_idle);
	}
	state = ThugState::jump;
	return THUG_BIND(update_jump);
}

fsm::StateFunction Thug::update_alert() { 
	animation.label = "alert";
	if (animation.just_started()) {
		if (m_services->random.percent_chance(50)) {
			m_services->soundboard.flags.thug.set(audio::Thug::alert_1);
		} else {
			m_services->soundboard.flags.thug.set(audio::Thug::alert_2);
		}
	}
	if (animation.complete()) {
		if (directions.actual.lr != directions.desired.lr) {
			state = ThugState::turn;
			animation.set_params(turn);
			return THUG_BIND(update_turn);
		}
		state = ThugState::rush;
		animation.set_params(rush);
		return THUG_BIND(update_rush);
	}
	state = ThugState::alert;
	return THUG_BIND(update_alert);
}

fsm::StateFunction Thug::update_rush() {
	if (change_state(ThugState::turn, turn)) { return THUG_BIND(update_turn); }
	if (caution.danger()) {
		state = ThugState::idle;
		animation.set_params(idle);
		return THUG_BIND(update_idle);
	}
	auto force{16.f};
	force *= directions.actual.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({force, 0.f});
	if (animation.complete()) {
		state = ThugState::idle;
		animation.set_params(idle);
		return THUG_BIND(update_idle);
	};
	state = ThugState::rush;
	return THUG_BIND(update_rush);
}

fsm::StateFunction Thug::update_punch() { return THUG_BIND(update_idle); }

bool Thug::change_state(ThugState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params, false);
		return true;
	}
	return false;
}

} // namespace enemy