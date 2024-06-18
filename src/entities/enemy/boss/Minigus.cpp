#include "Minigus.hpp"
#include "../../../level/Map.hpp"
#include "../../../gui/Console.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {
Minigus::~Minigus() {
	voice = {};
	sounds = {};
}
Minigus::Minigus(automa::ServiceProvider& svc, world::Map& map, gui::Console& console)
	: Enemy(svc, "minigus"), gun(svc, "minigun", 6), m_services(&svc), npc::NPC(svc, 7), m_map(&map), m_console(&console), health_bar(svc), sparkler(svc, Enemy::collider.vicinity.dimensions, flcolor::ui_white, "minigus") {
	animation.set_params(idle);
	gun.clip_cooldown_time = 360;
	gun.get().projectile.team = arms::TEAMS::SKYCORPS;
	gun.cycle.set_order(3);
	Enemy::collider.physics.maximum_velocity = {8.f, 18.f};
	Enemy::collider.physics.set_constant_friction({0.97f, 0.989f});
	cooldowns.vulnerability.start();
	cooldowns.exit.start();
	afterlife = 2000;

	NPC::dimensions = {24.f, 80.f};
	state_flags.set(npc::NPCState::force_interact);
	flags.general.set(GeneralFlags::post_death_render);
	
	secondary_collider = shape::Collider({48.f, 36.f});
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
	Enemy::collider.stats.GRAV = 6.0f;
	pre_direction.lr = dir::LR::left;
	post_direction.lr = dir::LR::left;
	sprite_direction.lr = dir::LR::left;
	Enemy::direction.lr = dir::LR::left;

	push_conversation("1");

	voice.hurt_1.setBuffer(svc.assets.b_minigus_hurt_1);
	voice.hurt_2.setBuffer(svc.assets.b_minigus_hurt_2);
	voice.hurt_3.setBuffer(svc.assets.b_minigus_hurt_3);
	voice.laugh_1.setBuffer(svc.assets.b_minigus_laugh);
	voice.laugh_2.setBuffer(svc.assets.b_minigus_laugh_2);
	voice.aww.setBuffer(svc.assets.b_minigus_aww);
	voice.babyimhome.setBuffer(svc.assets.b_minigus_babyimhome);
	voice.deepspeak.setBuffer(svc.assets.b_minigus_deepspeak);
	voice.doge.setBuffer(svc.assets.b_minigus_doge);
	voice.dontlookatme.setBuffer(svc.assets.b_minigus_dontlookatme);
	voice.exhale.setBuffer(svc.assets.b_minigus_exhale);
	voice.getit.setBuffer(svc.assets.b_minigus_getit);
	voice.greatidea.setBuffer(svc.assets.b_minigus_greatidea);
	voice.greatidea.setVolume(30);
	voice.itsagreatday.setBuffer(svc.assets.b_minigus_itsagreatday);
	voice.long_death.setBuffer(svc.assets.b_minigus_long_death);
	voice.long_moan.setBuffer(svc.assets.b_minigus_long_moan);
	voice.momma.setBuffer(svc.assets.b_minigus_momma);
	voice.mother.setBuffer(svc.assets.b_minigus_mother);
	voice.ok_1.setBuffer(svc.assets.b_minigus_ok_1);
	voice.ok_2.setBuffer(svc.assets.b_minigus_ok_2);
	voice.pizza.setBuffer(svc.assets.b_minigus_pizza);
	voice.poh.setBuffer(svc.assets.b_minigus_poh);
	voice.quick_breath.setBuffer(svc.assets.b_minigus_quick_breath);
	voice.thatisverysneeze.setBuffer(svc.assets.b_minigus_thatisverysneeze);
	voice.whatisit.setBuffer(svc.assets.b_minigus_whatisit);
	voice.woob.setBuffer(svc.assets.b_minigus_woob);

	sounds.land.setBuffer(svc.assets.b_heavy_land);
	sounds.crash.setBuffer(svc.assets.b_delay_crash);
	sounds.snap.setBuffer(svc.assets.sharp_click_buffer);
	sounds.lose_inv.setBuffer(svc.assets.b_laser);
	sounds.charge.setBuffer(svc.assets.b_gun_charge);
	sounds.inv.setBuffer(svc.assets.b_minigus_invincibility);
	sounds.build.setBuffer(svc.assets.b_minigus_build);

	sparkler.set_dimensions(Enemy::collider.vicinity.dimensions);
}

void Minigus::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	sparkler.update(svc);
	sparkler.set_position(Enemy::collider.vicinity.position);

	health_bar.update(svc, Enemy::health.get_max(), Enemy::health.get_hp());
	if (map.off_the_bottom(Enemy::collider.physics.position)) {
		post_death.cancel();
		flags.general.set(GeneralFlags::no_loot);
	}

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

	auto scl = sf::Vector2<float>{1.0f, 1.0f};
	sprite_direction.lr = Enemy::sprite.getScale() == scl ? dir::LR::left : dir::LR::right;

	gun.update(svc, map, *this);
	caution.avoid_ledges(map, Enemy::collider, 1);
	cooldowns.firing.update();
	cooldowns.post_charge.update();
	cooldowns.post_punch.update();
	cooldowns.hurt.update();
	cooldowns.player_punch.update();
	if (status.test(MinigusFlags::exit_scene)) { cooldowns.exit.update(); }
	if (state_flags.test(npc::NPCState::introduced)) { cooldowns.vulnerability.update(); }

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
		player.hurt(1);
		auto sign = Enemy::direction.lr == dir::LR::left ? -1.f : 1.f;
		player.accumulated_forces.push_back({sign * 10.f, -4.f});
		attacks.punch.sensor.deactivate();
		cooldowns.player_punch.start();
	}
	if (animation.get_frame() == 37 && attacks.uppercut.hit.active() && !cooldowns.player_punch.running()) {
		player.hurt(1);
		auto sign = Enemy::direction.lr == dir::LR::left ? -1.f : 1.f;
		player.accumulated_forces.push_back({sign * 10.f, -4.f});
		attacks.uppercut.sensor.deactivate();
		cooldowns.player_punch.start();
	}
	for (auto& index : map.collidable_indeces) {
		auto& cell = map.layers.at(world::MIDDLEGROUND).grid.cells.at(index);
		if (Enemy::collider.jumpbox.overlaps(cell.bounding_box) && cell.is_breakable()) { map.handle_breakables(cell, {}, 4); }
	}

	minigun.animation.update();
	if (minigun.sprite.getScale() != Enemy::sprite.getScale()) {
		minigun.direction = Enemy::direction;
		minigun.sprite.setScale(Enemy::sprite.getScale());
	}
	auto gun_base = Enemy::collider.physics.position + Enemy::collider.dimensions * 0.5f;
	auto gun_point = Enemy::direction.lr == dir::LR::left ? gun_base - sf::Vector2<float>{(float)minigun.dimensions.x, 0.f} : gun_base + sf::Vector2<float>{(float)minigun.dimensions.x, 0.f};
	gun.get().barrel_point = gun_point;

	Enemy::direction = post_direction;

	pre_direction.lr = (player.collider.physics.position.x + player.collider.bounding_box.dimensions.x * 0.5f < Enemy::collider.physics.position.x + Enemy::collider.dimensions.x * 0.5f) ? dir::LR::left : dir::LR::right;
	if (post_direction.lr != pre_direction.lr) {
		if (svc.ticker.every_x_ticks(100)) { 
			//std::cout << "mismatch\n";
		}
	}
	Enemy::update(svc, map, player);

	secondary_collider.physics.position = Enemy::collider.physics.position;
	secondary_collider.physics.position.y -= secondary_collider.dimensions.y;
	secondary_collider.physics.position.x += Enemy::direction.lr == dir::LR::left ? 0 :  Enemy::collider.dimensions.x - secondary_collider.dimensions.x;
	secondary_collider.sync_components();
	if (status.test(MinigusFlags::battle_mode)) { player.collider.handle_collider_collision(secondary_collider.bounding_box); }
	distant_range.set_position(Enemy::collider.bounding_box.position - (distant_range.dimensions * 0.5f) + (Enemy::collider.dimensions * 0.5f));
	player.collider.bounding_box.overlaps(distant_range) ? status.set(MinigusFlags::distant_range_activated) : status.reset(MinigusFlags::distant_range_activated);

	// state management

	state = MinigusState::idle;

	if (status.test(MinigusFlags::distant_range_activated) && !alert() && !hostile()) { state = MinigusState::run; }
	if (attacks.uppercut.sensor.active() && cooldowns.post_punch.is_complete()) { state = MinigusState::uppercut; }
	if (attacks.punch.sensor.active() && cooldowns.post_punch.is_complete()) { state = MinigusState::punch; }

	if (flags.state.test(StateFlags::hurt)) {
		cooldowns.hurt.start();
		if (svc.random.percent_chance(40)) {
			voice.hurt_1.play();
		} else if (svc.random.percent_chance(40)) {
			voice.hurt_2.play();
		} else {
			voice.hurt_3.play();
		}
		flags.state.reset(StateFlags::hurt);
	}

	if (just_died()) {}

	if (minigun.flags.test(MinigunFlags::exhausted) && cooldowns.firing.is_complete()) {
		if (svc.random.percent_chance(8)) { state = MinigusState::reload; }
	}

	if (gun.clip_cooldown.is_complete() && !minigun.flags.test(MinigunFlags::exhausted) && !cooldowns.post_charge.running() && hostile()) {
		if (m_services->random.percent_chance(rush_chance) && flags.state.test(StateFlags::vulnerable) && Enemy::collider.grounded()) { state = MinigusState::rush; }
		if (m_services->random.percent_chance(snap_chance) && !flags.state.test(StateFlags::vulnerable) && Enemy::collider.grounded() && !(counters.snap.get_count() > 1) && health.get_hp() < health.get_max() * 0.7f) {
			state = MinigusState::snap;
		}
		if (m_services->random.percent_chance(fire_chance)) { 
			if(health.get_hp() < health.get_max() * 0.7f) {
				if (m_services->random.percent_chance(60)) {
					state = MinigusState::jump_shoot;
				} else {
					state = MinigusState::shoot;
				}
			} else {
				state = MinigusState::shoot;
			}
		}
	}

	if (player.collider.bounding_box.overlaps(Enemy::collider.vicinity)) {
		if (m_services->random.percent_chance(30)) {
			state = MinigusState::run;
		} else {
			state = MinigusState::jumpsquat;
		}
	}

	if (alert() && Enemy::collider.grounded()) {
		if (!caution.danger(movement_direction) && cooldowns.post_punch.is_complete()) {
			state = MinigusState::run;
			if (attacks.uppercut.sensor.active()) { state = MinigusState::uppercut; }
			if (attacks.punch.sensor.active()) { state = MinigusState::punch; }
		} else {
			state = MinigusState::jumpsquat;
		}
	}

	if (caution.danger(movement_direction)) { state = MinigusState::jumpsquat; }

	if (Enemy::health_indicator.get_amount() < -80 && flags.state.test(StateFlags::vulnerable)) { state = MinigusState::build_invincibility; }
	if (cooldowns.vulnerability.is_complete() && flags.state.test(StateFlags::vulnerable)) { state = MinigusState::build_invincibility; }

	if (pre_direction.lr != post_direction.lr) { state = MinigusState::turn; }
	if (!(state == MinigusState::turn) && sprite_direction.lr != post_direction.lr) { Enemy::sprite.scale({-1.f, 1.f}); }
	movement_direction.lr = Enemy::collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;

	if (!status.test(MinigusFlags::battle_mode)) { state = MinigusState::idle; }

	// NPC stuff
	
	if (player.collider.bounding_box.overlaps(distant_range) && !state_flags.test(npc::NPCState::introduced) && state_flags.test(npc::NPCState::force_interact)) { triggers.set(npc::NPCTrigger::distant_interact); }

	NPC::update(svc, map, *m_console, player);
	if (player.transponder.get_voice_shipment() == 1) {
		voice.greatidea.play();
		player.transponder.flush_shipments();
	}
	if (player.transponder.get_voice_shipment() == 2) { voice.dontlookatme.play(); }
	if (player.transponder.get_voice_shipment() == 3) { voice.laugh_1.play(); }
	if (player.transponder.get_voice_shipment() == 4) { voice.pizza.play(); }
	if (state_flags.test(npc::NPCState::introduced) && !status.test(MinigusFlags::theme_song)) {
		svc.music.load("minigus");
		svc.music.play_looped(20);
		status.set(MinigusFlags::theme_song);
	}
	if (state_flags.test(npc::NPCState::introduced) && !status.test(MinigusFlags::battle_mode) && m_console->is_complete() && !health_bar.empty() && !status.test(MinigusFlags::exit_scene)) {
		status.set(MinigusFlags::battle_mode);
		triggers.reset(npc::NPCTrigger::distant_interact);
		player.transponder.flush_shipments();
		svc.music.load("scuffle");
		svc.music.play_looped(30);
		cooldowns.vulnerability.start();
	// start battle music!{
	}

	if (health_bar.empty() && !status.test(MinigusFlags::over_and_out) && !status.test(MinigusFlags::goodbye)) { state = MinigusState::struggle; }

	if (player.transponder.get_quest_shipment() == 3 && status.test(MinigusFlags::goodbye)) { status.set(MinigusFlags::over_and_out); }

	state_function = state_function();
}

void Minigus::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	NPC::render(svc, win, cam);
	auto u = minigun.animation.get_frame() >= 13 ? 78 : 0;
	auto v = (minigun.animation.get_frame() % 13) * 30;
	minigun.sprite.setTextureRect(sf::IntRect({{u, v}, minigun.dimensions}));
	minigun.sprite.setPosition(Enemy::sprite.getPosition() + minigun.offset);
	Enemy::sprite.setTexture(flags.state.test(StateFlags::vulnerable) ? svc.assets.t_minigus : svc.assets.t_minigus_inv);
	sparkler.render(svc, win, cam);
	if (cooldowns.hurt.running() && flags.state.test(StateFlags::vulnerable) && !(state == MinigusState::build_invincibility)) {
		Enemy::sprite.setTexture(hurt_color.get_alternator() % 2 == 0 ? svc.assets.t_minigus_blue : svc.assets.t_minigus_red);
	}
	if (!svc.greyblock_mode()) {
		win.draw(minigun.sprite);
	} else {
		secondary_collider.render(win, cam);
		attacks.punch.render(win, cam);
		attacks.uppercut.render(win, cam);
	}
	//attacks.left_shockwave.render(win, cam);
	//attacks.right_shockwave.render(win, cam);
}

void Minigus::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (status.test(MinigusFlags::battle_mode)) { health_bar.render(win); }
}

fsm::StateFunction Minigus::update_idle() {
	if (animation.just_started() && anim_debug) { std::cout << "idle\n"; }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
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
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (animation.just_started()) {
		voice.doge.play();
		minigun.animation.set_params(minigun.charging);
		minigun.flags.set(MinigunFlags::charging);
		sounds.charge.play();
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

		if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }

		if (m_services->random.percent_chance(50)) {
			state = MinigusState::run;
			animation.set_params(run);
			return MINIGUS_BIND(update_run);
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
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
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
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
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
	if (animation.just_started()) { voice.woob.play(); }
	cooldowns.jump.update();
	if (animation.just_started()) { cooldowns.jump.start(); }
	auto sign = Enemy::direction.lr == dir::LR::left ? -1.f : 1.f;
	if (status.test(MinigusFlags::over_and_out)) { sign = 0; }
	if (cooldowns.jump.running()) { Enemy::collider.physics.apply_force({sign * 36.f, -8.f}); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (cooldowns.jump.is_complete() && status.test(MinigusFlags::over_and_out)) { flags.general.reset(GeneralFlags::map_collision); }
	if (Enemy::collider.grounded() && cooldowns.jump.is_complete()) {
		m_map->shake_camera();
		sounds.land.play();
		sounds.crash.play();
		attacks.left_shockwave.start();
		attacks.right_shockwave.start();
		if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::jump;
	return MINIGUS_BIND(update_jump);
}

fsm::StateFunction Minigus::update_jump_shoot() {
	if (animation.just_started() && anim_debug) { std::cout << "jump_shoot\n"; }
	if (animation.just_started()) { voice.getit.play(); }
	if (minigun.animation.complete() && minigun.flags.test(MinigunFlags::charging)) {}
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (cooldowns.pre_jump.get_cooldown() != -1) { cooldowns.pre_jump.update(); }
	cooldowns.jump.update();
	auto sign = Enemy::direction.lr == dir::LR::left ? 1.f : -2.f;
	if (cooldowns.jump.running()) {
		Enemy::collider.physics.apply_force({sign * 4.f, -8.f});
	}
	if (animation.just_started()) {
		cooldowns.pre_jump.start();
		minigun.animation.set_params(minigun.charging);
		minigun.flags.set(MinigunFlags::charging);
		sounds.charge.play();
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

		if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }

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
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (animation.complete()) {
		minigun.flags.reset(MinigunFlags::exhausted);
		minigun.animation.set_params(minigun.neutral);
		voice.deepspeak.play();
		if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
		if (m_services->random.percent_chance(40)) {
			state = MinigusState::laugh;
			animation.set_params(laugh);
			return MINIGUS_BIND(update_laugh);
		} else {
			state = MinigusState::run;
			animation.set_params(run);
			return MINIGUS_BIND(update_run);
		}
	}
	state = MinigusState::reload;
	return MINIGUS_BIND(update_reload);
}

fsm::StateFunction Minigus::update_turn() {
	if (animation.just_started() && anim_debug) { std::cout << "turn\n"; }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
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
	auto sign = Enemy::direction.lr == dir::LR::left ? -1 : 1;
	Enemy::collider.physics.apply_force({Enemy::attributes.speed * sign, 0.f});
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
	if (change_state(MinigusState::punch, punch)) { return MINIGUS_BIND(update_punch); }
	if (change_state(MinigusState::uppercut, uppercut)) { return MINIGUS_BIND(update_uppercut); }
	if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
	if (animation.complete()) {
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::rush, rush)) { return MINIGUS_BIND(update_rush); }
		if (change_state(MinigusState::snap, snap)) { return MINIGUS_BIND(update_snap); }
		if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
		if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::punch, punch)) { return MINIGUS_BIND(update_punch); }
		if (change_state(MinigusState::uppercut, uppercut)) { return MINIGUS_BIND(update_uppercut); }
		if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::run;
	return MINIGUS_BIND(update_run);
}

fsm::StateFunction Minigus::update_punch() {
	if (animation.just_started() && anim_debug) { std::cout << "punch\n"; }
	if (animation.just_started()) { voice.mother.play(); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (animation.complete()) {
		cooldowns.post_punch.start();
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::rush, rush)) { return MINIGUS_BIND(update_rush); }
		if (change_state(MinigusState::snap, snap)) { return MINIGUS_BIND(update_snap); }
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
	if (animation.just_started()) { voice.momma.play(); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (animation.complete()) {
		cooldowns.post_punch.start();
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::rush, rush)) { return MINIGUS_BIND(update_rush); }
		if (change_state(MinigusState::snap, snap)) { return MINIGUS_BIND(update_snap); }
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
	if (animation.just_started()) {
		voice.grunt.play();
		sounds.build.play();
		sparkler.set_rate(4.f);
	}
	if (change_state(MinigusState::struggle, struggle)) {
		sparkler.set_rate(0.f);
		return MINIGUS_BIND(update_struggle);
	}
	cooldowns.hurt.cancel();
	if (animation.complete()) {
		flags.state.reset(StateFlags::vulnerable);
		counters.snap.start();
		sounds.inv.play();
		Enemy::sprite.setTexture(m_services->assets.t_minigus_inv);
		sparkler.set_rate(0.f);
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
			//m_services->soundboard.flags.minigus.set(audio::Minigus::laugh);
			voice.laugh_1.play();
		} else {
			//m_services->soundboard.flags.minigus.set(audio::Minigus::laugh_2);
			voice.laugh_2.play();
		}
	}
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
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
	if (animation.just_started()) { sounds.snap.play(); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
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
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	auto sign = Enemy::direction.lr == dir::LR::left ? -1.f : 1.f;
	Enemy::collider.physics.apply_force({sign * Enemy::attributes.speed * rush_speed, 0.f});
	if (cooldowns.rush.is_complete()) {
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::rush;
	return MINIGUS_BIND(update_rush);
}

fsm::StateFunction Minigus::update_struggle() {
	if (animation.just_started() && anim_debug) { std::cout << "struggle\n"; }
	if (animation.just_started()) {
		status.reset(MinigusFlags::battle_mode);
		triggers.set(npc::NPCTrigger::distant_interact);
		flush_conversations();
		push_conversation("2");
		m_services->music.stop();
		sounds.crash.play();	
		voice.long_moan.play();
	}
	auto randx = m_services->random.random_range_float(0.f, 60.f);
	auto randy = m_services->random.random_range_float(0.f, 60.f);
	sf::Vector2<float> pos = secondary_collider.physics.position + sf::Vector2<float>{randx, randy};
	if (m_services->ticker.every_x_ticks(80)) { m_map->effects.push_back(entity::Effect(*m_services, pos, {}, 0, 0)); }
	post_death.start(afterlife);
	Enemy::shake();
	Enemy::sprite_shake(*m_services, 20, 8);

	if (!animation.just_started() && m_console->is_complete() && !status.test(MinigusFlags::exit_scene)) {
		status.set(MinigusFlags::exit_scene);
		cooldowns.exit.start();
	}

	if (cooldowns.exit.is_complete() && status.test(MinigusFlags::exit_scene)) {
		triggers.set(npc::NPCTrigger::distant_interact);
		m_services->music.stop();
		flush_conversations();
		push_conversation("3");
		status.reset(MinigusFlags::exit_scene);
		status.set(MinigusFlags::goodbye);
		stop_shaking();
		state = MinigusState::exit;
		animation.set_params(idle);
		return MINIGUS_BIND(update_exit);
	}

	state = MinigusState::struggle;
	return MINIGUS_BIND(update_struggle);
}

fsm::StateFunction Minigus::update_exit() {
	if (animation.just_started() && anim_debug) { std::cout << "exit\n"; }
	if (status.test(MinigusFlags::over_and_out) && m_console->is_complete()) {
		m_map->active_loot.push_back(item::Loot(*m_services, get_attributes().drop_range, get_attributes().loot_multiplier, get_collider().bounding_box.position));
		state = MinigusState::jumpsquat;
		animation.set_params(jumpsquat);
		m_services->music.load("dusken_cove");
		m_services->music.play_looped(30);
		return MINIGUS_BIND(update_jumpsquat);
	}
	state = MinigusState::exit;
	return MINIGUS_BIND(update_exit);
}

bool Minigus::change_state(MinigusState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace enemy