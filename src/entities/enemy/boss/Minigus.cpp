#include "Minigus.hpp"
#include "../../../level/Map.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Minigus::Minigus(automa::ServiceProvider& svc, world::Map& map, gui::Console& console) : Enemy(svc, "minigus"), gun(svc, "minigun", 6), m_services(&svc), npc::NPC(svc, 7), m_map(&map), m_console(&console), health_bar(svc) {
	animation.set_params(idle);
	gun.clip_cooldown_time = 360;
	gun.get().projectile.team = arms::TEAMS::SKYCORPS;
	gun.cycle.set_order(3);
	Enemy::collider.physics.maximum_velocity = {3.f, 12.f};
	Enemy::collider.physics.air_friction = {0.95f, 0.999f};
	minigun.sprite.setTexture(svc.assets.t_minigun);
	minigun.sprite.setOrigin({(float)minigun.dimensions.x, minigun.dimensions.y * 0.8f});
	minigun.animation.set_params(minigun.neutral);
	flags.state.set(StateFlags::vulnerable);

	attacks.punch.sensor.bounds.setRadius(60);
	attacks.punch.hit.bounds.setRadius(20);
	attacks.punch.origin = {-10.f, -26.f};

	attacks.uppercut.sensor.bounds.setRadius(60);
	attacks.uppercut.hit.bounds.setRadius(20);
	attacks.uppercut.origin = {-8.f, 40.f};

	distant_range.dimensions = {900, 200};
	Enemy::collider.stats.GRAV = 2.0f;
	pre_direction.lr = dir::LR::left;
	post_direction.lr = dir::LR::left;
	Enemy::direction.lr = dir::LR::left;
	Enemy::direction.lr = dir::LR::left;
	cooldowns.vulnerability.start();
}

void Minigus::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	health_bar.update(svc, Enemy::health.get_max(), Enemy::health.get_hp());
	colliders.head.physics.position = Enemy::collider.physics.position;
	colliders.head.physics.position.y -= colliders.head.dimensions.y;

	if (Enemy::direction.lr == dir::LR::left) {
		attacks.punch.set_position(Enemy::collider.physics.position);
		attacks.uppercut.set_position(Enemy::collider.physics.position);
		attacks.punch.origin.x = -10.f;
		attacks.uppercut.origin.x = -8.f;
	} else {
		sf::Vector2<float> dir_offset{Enemy::collider.bounding_box.dimensions.x, 0.f};
		attacks.punch.set_position(Enemy::collider.physics.position + dir_offset);
		attacks.uppercut.set_position(Enemy::collider.physics.position + dir_offset);
		attacks.punch.origin.x = 10.f;
		attacks.uppercut.origin.x = 8.f;
	}

	gun.update(svc, map, *this);
	caution.avoid_ledges(map, Enemy::collider, 1);
	cooldowns.running_time.update();
	cooldowns.firing.update();
	cooldowns.post_charge.update();
	cooldowns.post_punch.update();
	cooldowns.hurt.update();
	cooldowns.player_punch.update();
	cooldowns.vulnerability.update();

	if (svc.ticker.every_x_ticks(32)) { hurt_color.update(); }
	
	attacks.punch.update();
	attacks.uppercut.update();
	attacks.punch.handle_player(player);
	attacks.uppercut.handle_player(player);

	attacks.left_shockwave.origin = Enemy::collider.physics.position + sf::Vector2<float>{0.f, Enemy::collider.bounding_box.dimensions.y};
	attacks.right_shockwave.origin = Enemy::collider.physics.position + Enemy::collider.bounding_box.dimensions;
	attacks.left_shockwave.update(svc, map);
	attacks.right_shockwave.update(svc, map);

	attacks.left_shockwave.handle_player(player);
	attacks.right_shockwave.handle_player(player);

	if (attacks.left_shockwave.hit.active() && !cooldowns.player_punch.running()) {
		player.hurt(1);
		if (!player.invincible()) { player.accumulated_forces.push_back({-40.f, -4.f}); }
		attacks.left_shockwave.hit.deactivate();
		cooldowns.player_punch.start();
	}
	if (attacks.right_shockwave.hit.active() && !cooldowns.player_punch.running()) {
		player.hurt(1);
		if (!player.invincible()) { player.accumulated_forces.push_back({40.f, -4.f}); }
		attacks.right_shockwave.hit.deactivate();
		cooldowns.player_punch.start();
	}

	if (animation.get_frame() == 30 && attacks.punch.hit.active() && !cooldowns.player_punch.running()) {
		player.hurt(2);
		auto sign = Enemy::direction.lr == dir::LR::left ? -1.f : 1.f;
		player.accumulated_forces.push_back({sign * 10.f, -4.f});
		attacks.punch.sensor.deactivate();
		cooldowns.player_punch.start();
	}
	if (animation.get_frame() == 37 && attacks.uppercut.hit.active() && !cooldowns.player_punch.running()) {
		player.hurt(2);
		auto sign = Enemy::direction.lr == dir::LR::left ? -1.f : 1.f;
		player.accumulated_forces.push_back({sign * 10.f, -4.f});
		attacks.uppercut.sensor.deactivate();
		cooldowns.player_punch.start();
	}

	minigun.animation.update();
	if (minigun.sprite.getScale() != Enemy::sprite.getScale()) {
		minigun.direction = Enemy::direction;
		minigun.sprite.setScale(Enemy::sprite.getScale());
	}
	auto gun_base = Enemy::collider.physics.position + Enemy::collider.dimensions * 0.5f;
	auto gun_point = Enemy::direction.lr == dir::LR::left ? gun_base - sf::Vector2<float>{(float)minigun.dimensions.x, 0.f} : gun_base + sf::Vector2<float>{(float)minigun.dimensions.x, 0.f};
	gun.get().barrel_point = gun_point;

	state = MinigusState::idle;

	Enemy::direction = post_direction;

	pre_direction.lr = (player.collider.physics.position.x + player.collider.bounding_box.dimensions.x * 0.5f < Enemy::collider.physics.position.x + Enemy::collider.dimensions.x * 0.5f) ? dir::LR::left : dir::LR::right;
	
	Enemy::update(svc, map, player);
	distant_range.set_position(Enemy::collider.bounding_box.position - (Enemy::physical.alert_range.dimensions * 0.5f) + (Enemy::collider.dimensions * 0.5f));
	player.collider.bounding_box.overlaps(distant_range) ? status.set(MinigusFlags::distant_range_activated) : status.reset(MinigusFlags::distant_range_activated);

	if (caution.danger(Enemy::direction)) { cooldowns.running_time.cancel(); }
	if (cooldowns.running_time.is_complete() && gun.clip_cooldown.is_complete()) {
		state = MinigusState::idle;
	} else if (!cooldowns.running_time.is_complete()) {
		state = MinigusState::run;
	}

	if (cooldowns.running_time.is_complete()) { state = MinigusState::idle; }

	if (status.test(MinigusFlags::distant_range_activated) && !alert() && !hostile()) {
		state = MinigusState::run;
	}

	if (attacks.punch.sensor.active() && cooldowns.post_punch.is_complete()) { state = MinigusState::punch; }
	if (attacks.uppercut.sensor.active() && cooldowns.post_punch.is_complete()) { state = MinigusState::uppercut; }

	if (flags.state.test(StateFlags::hurt)) {
		cooldowns.hurt.start();
		if (svc.random.percent_chance(40)) {
			svc.soundboard.flags.minigus.set(audio::Minigus::hurt_1);
		} else if (svc.random.percent_chance(40)) {
			svc.soundboard.flags.minigus.set(audio::Minigus::hurt_2);
		} else {
			svc.soundboard.flags.minigus.set(audio::Minigus::hurt_3);
		}
		flags.state.reset(StateFlags::hurt);
	}

	if (just_died()) {}

	if (minigun.flags.test(MinigunFlags::exhausted) && cooldowns.firing.is_complete()) {
		if (svc.random.percent_chance(8)) { state = MinigusState::reload; }
	}

	if (gun.clip_cooldown.is_complete() && !minigun.flags.test(MinigunFlags::exhausted) && !cooldowns.post_charge.running() && hostile()) {
		if (m_services->random.percent_chance(rush_chance) && flags.state.test(StateFlags::vulnerable) && Enemy::collider.grounded()) { state = MinigusState::rush; }
		if (m_services->random.percent_chance(snap_chance) && !flags.state.test(StateFlags::vulnerable) && Enemy::collider.grounded() && !(counters.snap.get_count() > 1) && health.get_hp() < health.get_max() * 0.5f) {
			state = MinigusState::snap;
		}
		if (m_services->random.percent_chance(fire_chance)) { 
			if(health.get_hp() < health.get_max() * 0.5f) {
				if (m_services->random.percent_chance(80)) {
					state = MinigusState::jump_shoot;
				} else {
					state = MinigusState::shoot;
				}
			} else {
				state = MinigusState::shoot;
			}
		}
	}

	if (player.collider.bounding_box.overlaps(Enemy::collider.vicinity) && !cooldowns.running_time.running()) {
		if (m_services->random.percent_chance(50)) {
			state = MinigusState::run;
			cooldowns.running_time.start();
		} else {
			state = MinigusState::jumpsquat;
		}
	}

	if (alert() && Enemy::collider.grounded()) {
		if(!caution.danger(movement_direction) && cooldowns.post_punch.is_complete()) {
			state = MinigusState::run;
			if (attacks.punch.sensor.active()) { state = MinigusState::punch; }
			if (attacks.uppercut.sensor.active()) { state = MinigusState::uppercut; }
		} else {
			state = MinigusState::jumpsquat;
		}
	}
	if (caution.danger(movement_direction)) { state = MinigusState::jumpsquat; }

	if (Enemy::health_indicator.get_amount() < -80 && flags.state.test(StateFlags::vulnerable)) { state = MinigusState::build_invincibility; }
	if (cooldowns.vulnerability.is_complete() && flags.state.test(StateFlags::vulnerable)) { state = MinigusState::build_invincibility; }

	if (pre_direction.lr != post_direction.lr) { state = MinigusState::turn; }
	movement_direction.lr = Enemy::collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;

	state_function = state_function();
}

void Minigus::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	auto u = minigun.animation.get_frame() >= 13 ? 78 : 0;
	auto v = (minigun.animation.get_frame() % 13) * 30;
	minigun.sprite.setTextureRect(sf::IntRect({{u, v}, minigun.dimensions}));
	minigun.sprite.setPosition(Enemy::sprite.getPosition() + minigun.offset);
	Enemy::sprite.setTexture(flags.state.test(StateFlags::vulnerable) ? svc.assets.t_minigus : svc.assets.t_minigus_inv);
	if (cooldowns.hurt.running() && flags.state.test(StateFlags::vulnerable) && !(state == MinigusState::build_invincibility)) {
		Enemy::sprite.setTexture(hurt_color.get_alternator() % 2 == 0 ? svc.assets.t_minigus_blue : svc.assets.t_minigus_red);
	}
	if (!svc.greyblock_mode()) {
		win.draw(minigun.sprite);
	} else {
		attacks.punch.render(win, cam);
		attacks.uppercut.render(win, cam);
	}
	attacks.left_shockwave.render(win, cam);
	attacks.right_shockwave.render(win, cam);
}

void Minigus::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { health_bar.render(win); }

fsm::StateFunction Minigus::update_idle() {
	if (animation.just_started() && anim_debug) { std::cout << "idle\n"; }
	if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
	if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
	if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
	if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
	if (change_state(MinigusState::jump_shoot, jump_shoot)) { return MINIGUS_BIND(update_jump_shoot); }
	if (change_state(MinigusState::punch, punch)) { return MINIGUS_BIND(update_punch); }
	if (change_state(MinigusState::uppercut, uppercut)) { return MINIGUS_BIND(update_uppercut); }
	if (change_state(MinigusState::snap, snap)) { return MINIGUS_BIND(update_snap); }
	if (change_state(MinigusState::rush, rush)) { return MINIGUS_BIND(update_rush); }
	if (change_state(MinigusState::build_invincibility, build_invincibility)) { return MINIGUS_BIND(update_build_invincibility); }
	if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
	state = MinigusState::idle;
	return MINIGUS_BIND(update_idle);
}

fsm::StateFunction Minigus::update_shoot() {
	if (animation.just_started() && anim_debug) { std::cout << "shoot\n"; }
	if (animation.just_started()) {
		minigun.animation.set_params(minigun.charging);
		minigun.flags.set(MinigunFlags::charging);
	}
	if (!gun.get().cooling_down() && !minigun.flags.test(MinigunFlags::charging)) {
		gun.cycle.update();
		gun.barrel_offset = gun.cycle.get_alternator() % 2 == 0 ? sf::Vector2<float>{0.f, 10.f} : (gun.cycle.get_alternator() % 2 == 1 ? sf::Vector2<float>{0.f, 20.f} : sf::Vector2<float>{0.f, 15.f});
		gun.shoot();
		m_map->spawn_projectile_at(*m_services, gun.get(), gun.barrel_point());
		m_map->shake_camera();
		m_services->soundboard.flags.weapon.set(audio::Weapon::skycorps_ar);
	}
	if (minigun.animation.complete() && minigun.flags.test(MinigunFlags::charging)) {
		minigun.flags.reset(MinigunFlags::charging);
		minigun.animation.set_params(minigun.firing);
		cooldowns.post_charge.start();
	}
	if (minigun.animation.complete() && !minigun.flags.test(MinigunFlags::charging)) {
		minigun.flags.set(MinigunFlags::exhausted);
		minigun.animation.set_params(minigun.deactivated);
		if (!flags.state.test(StateFlags::vulnerable)) { cooldowns.vulnerability.start(); }
		flags.state.set(StateFlags::vulnerable);
		counters.snap.cancel();
		Enemy::sprite.setTexture(m_services->assets.t_minigus);
		cooldowns.firing.start();

		if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }

		if (m_services->random.percent_chance(50)) {
			state = MinigusState::idle;
			animation.set_params(idle);
			return MINIGUS_BIND(update_idle);
		} else {
			state = MinigusState::laugh;
			animation.set_params(laugh);
			return MINIGUS_BIND(update_laugh);
		}
	}
	state = MinigusState::shoot;
	return MINIGUS_BIND(update_shoot);
}

fsm::StateFunction Minigus::update_jumpsquat() {
	if (animation.just_started() && anim_debug) { std::cout << "jumpsquat\n"; }
	if (animation.complete()) {
		state = MinigusState::jump;
		animation.set_params(jump);
		return MINIGUS_BIND(update_jump);
	}
	state = MinigusState::jumpsquat;
	return MINIGUS_BIND(update_jumpsquat);
}

fsm::StateFunction Minigus::update_hurt() {
	if (animation.just_started() && anim_debug) { std::cout << "hurt\n"; }
	if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
	if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
	if (animation.complete()) {
		flags.state.reset(StateFlags::hurt);
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::hurt;
	return MINIGUS_BIND(update_hurt);
}

fsm::StateFunction Minigus::update_jump() {
	if (animation.just_started() && anim_debug) { std::cout << "jump\n"; }
	cooldowns.jump.update();
	if (animation.just_started()) { cooldowns.jump.start(); }
	auto sign = Enemy::direction.lr == dir::LR::left ? -1.f : 1.f;
	if (cooldowns.jump.running()) { Enemy::collider.physics.apply_force({sign * 36.f, -4.f}); }
	if (Enemy::collider.grounded() && cooldowns.jump.is_complete()) {
		m_map->shake_camera();
		attacks.left_shockwave.start();
		attacks.right_shockwave.start();
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::jump;
	return MINIGUS_BIND(update_jump);
}

fsm::StateFunction Minigus::update_jump_shoot() {
	if (animation.just_started() && anim_debug) { std::cout << "jump_shoot\n"; }
	if (minigun.animation.complete() && minigun.flags.test(MinigunFlags::charging)) {  }
	if (cooldowns.pre_jump.get_cooldown() != -1) { cooldowns.pre_jump.update(); }
	cooldowns.jump.update();
	auto sign = Enemy::direction.lr == dir::LR::left ? 1.f : -2.f;
	if (cooldowns.jump.running()) {
		Enemy::collider.physics.apply_force({sign * 16.f, -4.f});
	}
	if (animation.just_started()) {
		cooldowns.pre_jump.start();
		minigun.animation.set_params(minigun.charging);
		minigun.flags.set(MinigunFlags::charging);
	}
	if (cooldowns.pre_jump.is_complete() && !cooldowns.jump.running()) {
		cooldowns.pre_jump.nullify();
		cooldowns.jump.start();
	}
	if (!gun.get().cooling_down() && !minigun.flags.test(MinigunFlags::charging)) {
		gun.cycle.update();
		gun.barrel_offset = gun.cycle.get_alternator() % 2 == 0 ? sf::Vector2<float>{0.f, 10.f} : (gun.cycle.get_alternator() % 2 == 1 ? sf::Vector2<float>{0.f, 20.f} : sf::Vector2<float>{0.f, 15.f});
		gun.shoot();
		m_map->spawn_projectile_at(*m_services, gun.get(), gun.barrel_point());
		m_map->shake_camera();
		m_services->soundboard.flags.weapon.set(audio::Weapon::skycorps_ar);
	}
	if (minigun.animation.complete() && minigun.flags.test(MinigunFlags::charging)) {
		minigun.flags.reset(MinigunFlags::charging);
		minigun.animation.set_params(minigun.firing);
		cooldowns.post_charge.start();
	}
	if (minigun.animation.complete() && !minigun.flags.test(MinigunFlags::charging)) {
		minigun.flags.set(MinigunFlags::exhausted);
		minigun.animation.set_params(minigun.deactivated);
		if (!flags.state.test(StateFlags::vulnerable)) { cooldowns.vulnerability.start(); }
		flags.state.set(StateFlags::vulnerable);
		counters.snap.cancel();
		Enemy::sprite.setTexture(m_services->assets.t_minigus);
		cooldowns.firing.start();

		if (m_services->random.percent_chance(50)) {
			state = MinigusState::idle;
			animation.set_params(idle);
			return MINIGUS_BIND(update_idle);
		} else {
			state = MinigusState::laugh;
			animation.set_params(laugh);
			return MINIGUS_BIND(update_laugh);
		}
	}
	state = MinigusState::jump_shoot;
	return MINIGUS_BIND(update_jump_shoot);
}

fsm::StateFunction Minigus::update_reload() {
	if (animation.just_started() && anim_debug) { std::cout << "reload\n"; }
	if (animation.complete()) {
		minigun.flags.reset(MinigunFlags::exhausted);
		minigun.animation.set_params(minigun.neutral);
		if (m_services->random.percent_chance(40)) {
			state = MinigusState::laugh;
			animation.set_params(laugh);
			return MINIGUS_BIND(update_laugh);
		} else {
			state = MinigusState::idle;
			animation.set_params(idle);
			return MINIGUS_BIND(update_idle);
		}
	}
	state = MinigusState::reload;
	return MINIGUS_BIND(update_reload);
}

fsm::StateFunction Minigus::update_turn() {
	if (animation.just_started() && anim_debug) { std::cout << "turn\n"; }
	if (animation.complete()) {
		Enemy::sprite.scale({-1.f, 1.f});
		post_direction = pre_direction;
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
	state = MinigusState::turn;
	return MINIGUS_BIND(update_turn);
}

fsm::StateFunction Minigus::update_run() {
	if (animation.just_started() && anim_debug) { std::cout << "run\n"; }
	if (animation.just_started()) { cooldowns.running_time.start(); }
	auto sign = Enemy::direction.lr == dir::LR::left ? 1 : -1;
	Enemy::collider.physics.apply_force({Enemy::attributes.speed * sign, 0.f});
	if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
	if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
	if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
	if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
	if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
	if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
	state = MinigusState::run;
	return MINIGUS_BIND(update_run);
}

fsm::StateFunction Minigus::update_punch() {
	if (animation.just_started() && anim_debug) { std::cout << "punch\n"; }
	if (animation.complete()) {
		cooldowns.post_punch.start();
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::punch;
	return MINIGUS_BIND(update_punch);
}

fsm::StateFunction Minigus::update_uppercut() {
	if (animation.just_started() && anim_debug) { std::cout << "uppercut\n"; }
	if (animation.complete()) {
		cooldowns.post_punch.start();
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::uppercut;
	return MINIGUS_BIND(update_uppercut);
}

fsm::StateFunction Minigus::update_build_invincibility() {
	if (animation.just_started() && anim_debug) { std::cout << "build_invincibility\n"; }
	cooldowns.hurt.cancel();
	if (animation.complete()) {
		flags.state.reset(StateFlags::vulnerable);
		counters.snap.start();
		Enemy::sprite.setTexture(m_services->assets.t_minigus_inv);
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::build_invincibility;
	return MINIGUS_BIND(update_build_invincibility);
}

fsm::StateFunction Minigus::update_laugh() {
	if (animation.just_started() && anim_debug) { std::cout << "laugh\n"; }
	if (animation.just_started()) { 
		if (m_services->random.percent_chance(50)) {
			m_services->soundboard.flags.minigus.set(audio::Minigus::laugh);
		} else {
			m_services->soundboard.flags.minigus.set(audio::Minigus::laugh_2);
		}
	}
	if (change_state(MinigusState::punch, punch)) { return MINIGUS_BIND(update_punch); }
	if (change_state(MinigusState::uppercut, uppercut)) { return MINIGUS_BIND(update_uppercut); }
	if (animation.complete()) {
		if (change_state(MinigusState::snap, snap)) { return MINIGUS_BIND(update_snap); }
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::laugh;
	return MINIGUS_BIND(update_laugh);
}

fsm::StateFunction Minigus::update_snap() {
	if (animation.just_started() && anim_debug) { std::cout << "snap\n"; }
	if (animation.complete()) {
		for (int i{0}; i < 2; ++i) {
			auto randx = m_services->random.random_range_float(-80.f, 80.f);
			auto randy = m_services->random.random_range_float(-120.f, 40.f);
			sf::Vector2<float> rand_vec{randx, randy};
			sf::Vector2<float> spawn = Enemy::collider.physics.position + rand_vec;
			m_map->spawn_enemy(5, spawn);
		}
		counters.snap.update();
		state = MinigusState::laugh;
		animation.set_params(laugh);
		return MINIGUS_BIND(update_laugh);
	}
	state = MinigusState::snap;
	return MINIGUS_BIND(update_snap);
}

fsm::StateFunction Minigus::update_rush() {
	if (animation.just_started() && anim_debug) { std::cout << "rush\n"; }
	cooldowns.rush.update();
	if (animation.just_started()) { cooldowns.rush.start(); }
	auto sign = Enemy::direction.lr == dir::LR::left ? -1.f : 1.f;
	Enemy::collider.physics.apply_force({sign * Enemy::attributes.speed * 8.f, 0.f});
	if (cooldowns.rush.is_complete()) {
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::rush;
	return MINIGUS_BIND(update_rush);
}

bool Minigus::change_state(MinigusState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace enemy