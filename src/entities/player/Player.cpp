#include "Player.hpp"
#include "../../gui/Console.hpp"
#include "../../gui/InventoryWindow.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../level/Map.hpp"
#include "../item/Drop.hpp"

namespace player {

Player::Player(automa::ServiceProvider& svc) : arsenal(svc), m_services(&svc), health_indicator(svc), orb_indicator(svc), controller(svc), animation(*this) {}

void Player::init(automa::ServiceProvider& svc) {

	m_services = &svc;

	svc.data.load_player_params(*this);
	health_indicator.init(svc, 0);
	orb_indicator.init(svc, 1);
	tutorial.update(svc);

	health.set_invincibility(400);

	collider = shape::Collider(sf::Vector2<float>{PLAYER_WIDTH, PLAYER_HEIGHT}, sf::Vector2<float>{PLAYER_START_X, PLAYER_START_Y});
	hurtbox.dimensions = sf::Vector2<float>{12.f, 26.f};
	collider.physics = components::PhysicsComponent({physics_stats.ground_fric, physics_stats.ground_fric}, physics_stats.mass);

	collider.physics.set_constant_friction({physics_stats.ground_fric, physics_stats.air_fric});
	collider.collision_depths = util::CollisionDepth();
	// if (collider.collision_depths) { std::cout << "Depth instantiated.\n"; }

	anchor_point = {collider.physics.position.x + PLAYER_WIDTH / 2, collider.physics.position.y + PLAYER_HEIGHT / 2};

	antennae.push_back(vfx::Gravitator(collider.physics.position, svc.styles.colors.bright_orange, antenna_force));
	antennae.push_back(vfx::Gravitator(collider.physics.position, svc.styles.colors.bright_orange, antenna_force, {2.f, 4.f}));

	antennae[0].collider.physics = components::PhysicsComponent(sf::Vector2<float>{physics_stats.antenna_friction, physics_stats.antenna_friction}, 1.0f);
	antennae[0].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);
	antennae[1].collider.physics = components::PhysicsComponent(sf::Vector2<float>{physics_stats.antenna_friction, physics_stats.antenna_friction}, 1.0f);
	antennae[1].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);

	sprite_dimensions = {48.f, 48.f};

	// sprites
	sprite.setTexture(svc.assets.t_nani);

	texture_updater.load_base_texture(svc.assets.t_nani);
	texture_updater.load_pixel_map(svc.assets.t_palette_nani);
	catalog.categories.wardrobe.set_palette(svc.assets.t_palette_nani);
}

void Player::update(world::Map& map, gui::Console& console, gui::InventoryWindow& inventory_window) {
	caution.avoid_ledges(map, collider, controller.direction, 8);
	if (collider.collision_depths) { collider.collision_depths.value().reset(); }
	tutorial.update(*m_services);
	cooldowns.tutorial.update();

	invincible() ? collider.draw_hurtbox.setFillColor(m_services->styles.colors.red) : collider.draw_hurtbox.setFillColor(m_services->styles.colors.blue);

	collider.flags.general.set(shape::General::complex);
	if (!catalog.categories.abilities.has_ability(Abilities::dash)) { controller.nullify_dash(); }

	collider.physics.gravity = physics_stats.grav;
	collider.physics.maximum_velocity = physics_stats.maximum_velocity;
	collider.physics.ground_friction = {physics_stats.ground_fric, physics_stats.ground_fric};
	collider.physics.air_friction = {physics_stats.air_fric, physics_stats.air_fric};
	if (flags.state.test(State::crushed)) { collider.physics.gravity = 0.f; }

	update_direction();
	grounded() ? controller.ground() : controller.unground();

	update_transponder(console, inventory_window);
	if (!catalog.categories.abilities.has_ability(Abilities::double_jump)) { controller.get_jump().jump_counter.cancel(); }
	controller.update(*m_services);

	if (grounded()) { controller.reset_dash_count(); }

	// do this elsehwere later
	if (collider.flags.state.test(shape::State::just_landed)) { m_services->soundboard.flags.player.set(audio::Player::land); }
	collider.flags.state.reset(shape::State::just_landed);

	// player-controlled actions
	if (hotbar) { hotbar.value().switch_weapon(*m_services, static_cast<int>(controller.arms_switch())); }
	dash();
	jump(map);
	wallslide();
	shield();
	update_animation();
	update_sprite();

	// check keystate
	if (!controller.get_jump().jumpsquatting()) { walk(); }
	if (!controller.moving() && (!force_cooldown.running() || collider.world_grounded())) { collider.physics.acceleration.x = 0.0f; }

	// weapon
	if (controller.shot() || controller.arms_switch()) { animation.idle_timer.start(); }
	if (flags.state.test(State::impart_recoil) && arsenal) {
		if (controller.direction.und == dir::UND::down) { accumulated_forces.push_back({0.f, -equipped_weapon().attributes.recoil}); }
		if (controller.direction.und == dir::UND::up) { accumulated_forces.push_back({0.f, equipped_weapon().attributes.recoil}); }
		flags.state.reset(State::impart_recoil);
	}

	force_cooldown.update();
	for (auto& force : accumulated_forces) { collider.physics.apply_force(force); }
	accumulated_forces.clear();
	collider.physics.position += forced_momentum;
	if (controller.moving() || collider.has_horizontal_collision() || collider.flags.external_state.test(shape::ExternalState::vert_world_collision) || collider.world_grounded()) { forced_momentum = {}; }

	collider.update(*m_services);
	hurtbox.set_position(collider.hurtbox.position - sf::Vector2<float>{0.f, 14.f});
	health.update();
	health_indicator.update(*m_services, collider.physics.position);
	orb_indicator.update(*m_services, collider.physics.position);
	if (orb_indicator.active()) { health_indicator.shift(); }
	update_invincibility();
	update_weapon();
	catalog.update(*m_services);

	if (catalog.categories.abilities.has_ability(Abilities::dash)) {
		if (!(animation.state == AnimState::dash) && !controller.dash_requested()) {
			controller.stop_dashing();
			controller.cancel_dash_request();
			collider.flags.dash.reset(shape::Dash::dash_cancel_collision);
		}
	}

	if (animation.state == AnimState::slide && m_services->ticker.every_x_ticks(12)) { map.active_emitters.push_back(vfx::Emitter(*m_services, collider.jumpbox.position, collider.jumpbox.dimensions, "slide", m_services->styles.colors.ui_white, dir::Direction(dir::UND::up))); }
	
	update_antennae();
	if (is_dead()) {
		for (auto& a : antennae) { a.collider.detect_map_collision(map); }
	}
}

void Player::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {

	sf::Vector2<float> player_pos = apparent_position - campos;
	calculate_sprite_offset();

	if (flags.state.test(State::crushed)) { return; }

	//debug
	collider.colors.local = controller.can_jump() ? svc.styles.colors.green : svc.styles.colors.red;

	// dashing effect
	sprite.setPosition(sprite_position);
	if (svc.ticker.every_x_frames(8) && animation.state == AnimState::dash) { sprite_history.update(sprite); }
	if (svc.ticker.every_x_frames(8) && !(animation.state == AnimState::dash)) { sprite_history.flush(); }
	sprite_history.drag(win, campos);

	// get UV coords
	int u = (int)(animation.get_frame() / asset::NANI_SPRITESHEET_HEIGHT) * asset::NANI_SPRITE_WIDTH;
	int v = (int)(animation.get_frame() % asset::NANI_SPRITESHEET_HEIGHT) * asset::NANI_SPRITE_WIDTH;
	sprite.setTextureRect(sf::IntRect({u, v}, {asset::NANI_SPRITE_WIDTH, asset::NANI_SPRITE_WIDTH}));
	sprite.setOrigin(asset::NANI_SPRITE_WIDTH / 2, asset::NANI_SPRITE_WIDTH / 2);
	sprite.setPosition(sprite_position.x - campos.x, sprite_position.y - campos.y);

	if (arsenal && hotbar) {
		collider.flags.general.set(shape::General::complex);
		if (flags.state.test(State::show_weapon)) { equipped_weapon().render_back(svc, win, campos); }
	}

	if (svc.greyblock_mode()) {
		win.draw(sprite);
		collider.render(win, campos);
		sf::RectangleShape box{};
		box.setFillColor(sf::Color::Transparent);
		box.setOutlineColor(svc.styles.colors.green);
		box.setOutlineThickness(-1);
		box.setPosition(hurtbox.position - campos);
		box.setSize(hurtbox.dimensions);
		win.draw(box);
	} else {
		antennae[1].render(svc, win, campos, 1);
		win.draw(sprite);
		antennae[0].render(svc, win, campos, 1);
	}

	if (arsenal && hotbar) {
		if (flags.state.test(State::show_weapon)) { equipped_weapon().render(svc, win, campos); }
	}

	if (controller.get_shield().active() && catalog.categories.abilities.has_ability(Abilities::shield)) { controller.get_shield().render(*m_services, win, campos); }
}

void Player::render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	health_indicator.render(svc, win, cam);
	orb_indicator.render(svc, win, cam);
}

void Player::assign_texture(sf::Texture& tex) { sprite.setTexture(tex); }

void Player::update_animation() {

	flags.state.set(State::show_weapon);

	if (grounded()) {
		if (controller.inspecting()) { animation.state = AnimState::inspect; }
		if (!(animation.state == AnimState::land || animation.state == AnimState::rise)) {
			if (controller.inspecting()) { animation.state = AnimState::inspect; }
			if (controller.nothing_pressed() && !controller.dashing() && !(animation.state == AnimState::inspect) && !(animation.state == AnimState::sit)) { animation.state = AnimState::idle; }
			if (controller.moving() && !controller.dashing() && !controller.sprinting()) { animation.state = AnimState::run; }
			if (controller.moving() && controller.sprinting() && !controller.dashing() && !(animation.state == AnimState::sharp_turn)) { animation.state = AnimState::sprint; }
			if (animation.state == AnimState::sprint && controller.sliding() && controller.get_slide().can_begin()) { animation.state = AnimState::slide; }
			if (abs(collider.physics.velocity.x) > thresholds.stop && !controller.moving()) { animation.state = AnimState::stop; }
			handle_turning();
		}
	} else {
		if (collider.physics.velocity.y > -thresholds.suspend && collider.physics.velocity.y < thresholds.suspend) { animation.state = AnimState::suspend; }
	}

	if (collider.physics.velocity.y > thresholds.suspend && !grounded()) { animation.state = AnimState::fall; }
	if (collider.flags.animation.test(shape::Animation::just_landed)) {
		collider.flags.animation.reset(shape::Animation::just_landed);
		animation.state = AnimState::land;
	}

	if (catalog.categories.abilities.has_ability(Abilities::dash)) {
		if (controller.dashing() && controller.can_dash()) { animation.state = AnimState::dash; }
		if (controller.dash_requested()) {
			animation.state = AnimState::dash;
			flags.state.reset(State::show_weapon);
		}
	}
	if (catalog.categories.abilities.has_ability(Abilities::wall_slide)) {
		if (controller.get_wallslide().is_wallsliding()) { animation.state = AnimState::wallslide; }
	}
	if (controller.moving() && grounded() && controller.can_jump()) {
		if (collider.has_left_wallslide_collision() && controller.horizontal_movement() < 0.f) { cooldowns.push.update(); }
		if (collider.has_right_wallslide_collision() && controller.horizontal_movement() > 0.f) { cooldowns.push.update(); }
		if (cooldowns.push.is_complete() && (collider.has_right_wallslide_collision() || collider.has_left_wallslide_collision())) { animation.state = AnimState::push; }
	}
	if (catalog.categories.abilities.has_ability(Abilities::shield)) {
		if (controller.get_shield().is_shielding() && grounded() && !(animation.state == AnimState::land) && !(animation.state == AnimState::fall)) {
			animation.state = AnimState::shield;
			controller.prevent_movement();
			flags.state.reset(State::show_weapon);
		}
	}
	if (animation.state == AnimState::sit) { flags.state.reset(State::show_weapon); }
	if (hurt_cooldown.running()) { animation.state = AnimState::hurt; }
	if (is_dead()) {
		animation.state = AnimState::die;
		flags.state.reset(State::show_weapon);
	}

	//for sliding down ramps
	animation.state == AnimState::slide ? collider.flags.animation.set(shape::Animation::sliding) : collider.flags.animation.reset(shape::Animation::sliding);

	animation.update();
}

void Player::update_sprite() {

	if (animation.triggers.consume(AnimTriggers::flip)) {
		sprite.scale(-1.0f, 1.0f);
		if (animation.animation.label == "turn" || animation.animation.label == "sharp_turn") { animation.animation.set_params(idle); }
	}

	flags.state.reset(State::dir_switch);
	// flip the sprite based on the player's direction
	sf::Vector2<float> right_scale = {1.0f, 1.0f};
	sf::Vector2<float> left_scale = {-1.0f, 1.0f};
	if (!grounded()) {
		if (controller.facing_left() && sprite.getScale() == right_scale) { sprite.scale(-1.0f, 1.0f); }
		if (controller.facing_right() && sprite.getScale() == left_scale) { sprite.scale(-1.0f, 1.0f); }
	}

	// check for quick turn
	if (controller.quick_turn()) { flags.state.set(State::dir_switch); }

	sprite.setTexture(texture_updater.get_dynamic_texture());
}

void Player::handle_turning() {
	sf::Vector2<float> right_scale = {1.0f, 1.0f};
	sf::Vector2<float> left_scale = {-1.0f, 1.0f};
	if (controller.facing_left() && sprite.getScale() == right_scale) { animation.state = collider.physics.velocity.x > thresholds.quick_turn ? AnimState::sharp_turn : AnimState::turn; }
	if (controller.facing_right() && sprite.getScale() == left_scale) { animation.state = collider.physics.velocity.x < -thresholds.quick_turn ? AnimState::sharp_turn : AnimState::turn; }
}

void Player::update_transponder(gui::Console& console, gui::InventoryWindow& inventory_window) {
	if (inventory_window.active()) {
		controller.restrict_movement();
		controller.prevent_movement();
		if (controller.transponder_up()) { inventory_window.move(*this, {0, -1}, static_cast<bool>(arsenal)); }
		if (controller.transponder_down()) { inventory_window.move(*this, {0, 1}, static_cast<bool>(arsenal)); }
		if (controller.transponder_left()) { inventory_window.move(*this, {-1, 0}); }
		if (controller.transponder_right()) { inventory_window.move(*this, {1, 0}); }
		if (controller.transponder_hold_up() && inventory_window.is_minimap()) { inventory_window.minimap.move({0.f, -1.f}); }
		if (controller.transponder_hold_down() && inventory_window.is_minimap()) { inventory_window.minimap.move({0.f, 1.f}); }
		if (controller.transponder_hold_left() && inventory_window.is_minimap()) { inventory_window.minimap.move({-1.f, 0.f}); }
		if (controller.transponder_hold_right() && inventory_window.is_minimap()) { inventory_window.minimap.move({1.f, 0.f}); }
		if (controller.transponder_select() && inventory_window.is_minimap()) { inventory_window.minimap.toggle_scale(); }
		transponder.update(*m_services, inventory_window);
		if (tutorial.current_state == text::TutorialFlags::inventory) {
			tutorial.flags.set(text::TutorialFlags::inventory);
			tutorial.current_state = text::TutorialFlags::shoot;
			tutorial.turn_off();
		}
		if (inventory_window.is_minimap()) {
			if (tutorial.current_state == text::TutorialFlags::map) {
				tutorial.flags.set(text::TutorialFlags::map);
				tutorial.turn_off();
				tutorial.close_for_good();
			}
		}
	}
	if (console.active()) {
		controller.restrict_movement();
		controller.prevent_movement();
		if (controller.transponder_skip()) { transponder.skip_ahead(); }
		if (controller.transponder_skip_released()) { transponder.enable_skip(); }
		if (controller.transponder_next()) { transponder.next(); }
		if (controller.transponder_exit()) { transponder.exit(); }
		if (controller.transponder_up()) { transponder.go_up(); }
		if (controller.transponder_down()) { transponder.go_down(); }
		if (controller.transponder_select()) { transponder.select(); }
		transponder.update(*m_services, console);
		auto emotion = transponder.shipments.emotion.get_residue();
		if (console.portrait.get_emotion() != emotion && emotion != 0) { console.portrait.set_emotion(emotion); }
	}
	transponder.end();
	if (transponder.shipments.item.get_residue() > 0) {
		give_item(transponder.shipments.item.get_residue(), 1);
		console.display_item(transponder.shipments.item.get_residue());
	}
	auto qs = transponder.shipments.quest.consume_pulse();
	auto ri = transponder.shipments.reveal_item.consume_pulse();
	if (qs > 0) { /* do something with quest tracker */
		quest_code = util::QuestCode(qs);
		if (quest_code.value().destroy_inspectable()) { m_services->quest.progress(static_cast<fornani::QuestType>(transponder.out_quest.type), transponder.out_quest.id, 1); }
		if (ri > 0) { catalog.categories.inventory.reveal_item(ri); }
		// std::cout << "Transponded: " << transponder.out_quest.type << ", " << transponder.out_quest.id << ", " << transponder.out_quest.source_id << "\n";
		//  handle other quest code types
		quest_code = {};
		transponder.out_quest = {};
	}
}

void Player::flash_sprite() {
	if ((health.invincibility.get_cooldown() / 30) % 2 == 0) {
		sprite.setColor(m_services->styles.colors.red);
	} else {
		sprite.setColor(m_services->styles.colors.blue);
	}
}

void Player::calculate_sprite_offset() {
	sprite_position = {collider.get_average_tick_position() + sprite_offset};
	sprite_position.x += controller.facing_left() ? -1.f : 1.f;
	apparent_position = collider.get_average_tick_position() + collider.dimensions / 2.f;
}

void Player::set_idle() {
	animation.state = AnimState::idle;
	animation.animation.set_params(idle);
	animation.state_function = std::bind(&PlayerAnimation::update_idle, &animation);
}

void Player::jump(world::Map& map) {
	if (is_dead() || animation.state == AnimState::die) { return; }
	if (controller.get_jump().began()) {
		collider.flags.movement.set(shape::Movement::jumping);
		animation.state = AnimState::rise;
		if (m_services->ticker.every_x_ticks(20)) {
			map.active_emitters.push_back(vfx::Emitter(*m_services, collider.jumpbox.position, collider.jumpbox.dimensions, "jump", m_services->styles.colors.ui_white, dir::Direction(dir::UND::up)));
		}
		if (tutorial.current_state == text::TutorialFlags::jump) {
			tutorial.flags.set(text::TutorialFlags::jump);
			tutorial.current_state = text::TutorialFlags::sprint;
			tutorial.trigger();
			tutorial.turn_on();
		}
	} else {
		collider.flags.movement.reset(shape::Movement::jumping);
	}
	if (controller.get_jump().jumpsquat_trigger()) {
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.jumpbox.position, collider.jumpbox.dimensions, "jump", m_services->styles.colors.ui_white, dir::Direction(dir::UND::up)));
		controller.get_jump().start_jumpsquat();
		controller.get_jump().reset_jumpsquat_trigger();
		collider.flags.movement.set(shape::Movement::jumping);
	}
	if (controller.get_jump().jumpsquatting()) {
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.jumpbox.position, collider.jumpbox.dimensions, "jump", m_services->styles.colors.ui_white, dir::Direction(dir::UND::up)));
		controller.get_jump().stop_jumpsquatting();
		controller.get_jump().start();
		collider.physics.acceleration.y = -physics_stats.jump_velocity;
		animation.state = AnimState::rise;
		m_services->soundboard.flags.player.set(audio::Player::jump);
		collider.flags.movement.set(shape::Movement::jumping);
	} else if (controller.get_jump().released() && controller.get_jump().jumping() && !controller.get_jump().held() && collider.physics.velocity.y < 0) {
		collider.physics.acceleration.y *= physics_stats.jump_release_multiplier;
		controller.get_jump().reset();
	}
	if (collider.flags.state.test(shape::State::just_landed)) { controller.get_jump().reset_jumping(); }
	if (catalog.categories.abilities.has_ability(Abilities::double_jump)) {
		if (controller.get_jump().just_doublejumped()) {
			collider.physics.velocity.y = 0.f;
			controller.get_jump().doublejump();
			m_services->soundboard.flags.player.set(audio::Player::jump);
			map.effects.push_back(entity::Effect(*m_services, sprite_position, sf::Vector2<float>{collider.physics.velocity.x * 0.1f, 0.f}, 0, 9));
		}
		if (controller.get_jump().is_doublejump()) { animation.state = AnimState::backflip; }
	}
}

void Player::dash() {
	if (!catalog.categories.abilities.has_ability(Abilities::dash)) { return; }
	if (animation.state == AnimState::dash || controller.dash_requested()) {
		collider.flags.movement.set(shape::Movement::dashing);
		collider.physics.acceleration.y = controller.vertical_movement() * physics_stats.vertical_dash_multiplier;
		collider.physics.velocity.y = controller.vertical_movement() * physics_stats.vertical_dash_multiplier;

		if (!collider.flags.dash.test(shape::Dash::dash_cancel_collision)) {
			collider.physics.acceleration.x += controller.dash_value() * physics_stats.dash_speed;
			collider.physics.velocity.x += controller.dash_value() * physics_stats.dash_speed;
		}
		controller.dash();
	}
}

void Player::wallslide() {
	if (!catalog.categories.abilities.has_ability(Abilities::wall_slide)) { return; }
	controller.get_wallslide().end();
	if (!grounded() && collider.physics.velocity.y > thresholds.wallslide) {
		if ((collider.has_left_wallslide_collision() && controller.moving_left()) || (collider.has_right_wallslide_collision() && controller.moving_right())) {
			controller.get_wallslide().start();
			collider.physics.acceleration.y = std::min(collider.physics.acceleration.y, physics_stats.wallslide_speed);
		}
	}
}

void Player::shield() {
	if (!catalog.categories.abilities.has_ability(Abilities::shield)) {
		controller.get_shield().flags = {};
		return;
	}
	controller.get_shield().update(*m_services);
	controller.get_shield().sensor.bounds.setPosition(collider.bounding_box.position + collider.bounding_box.dimensions * 0.5f);
}

void Player::set_position(sf::Vector2<float> new_pos, bool centered) {
	sf::Vector2<float> offset{};
	offset.x = centered ? collider.dimensions.x * 0.5f : 0.f;
	collider.physics.position = new_pos - offset;
	collider.sync_components();
	update_direction();
	sync_antennae();
	health_indicator.set_position(new_pos);
	orb_indicator.set_position(new_pos);
	if (arsenal && hotbar) { equipped_weapon().set_position(new_pos); }
}

void Player::freeze_position() {
	set_position(collider.physics.previous_position);
	collider.physics.zero();
}

void Player::update_direction() {
	if (controller.facing_left()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2 - ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	} else if (controller.facing_right()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2 + ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	} else {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	}

	// set directions for grappling hook
	//if (arsenal) { equipped_weapon().projectile.hook.probe_direction = controller.direction; }
}

void Player::update_weapon() {
	if (!arsenal) { return; }
	if (!hotbar) { return; }
	// update all weapons in loadout to avoid unusual behavior upon fast weapon switching
	for (auto& weapon : arsenal.value().get_loadout()) {
		hotbar->has(weapon->get_id()) ? weapon->set_hotbar() : weapon->set_reserved();
		weapon->firing_direction = controller.direction;
		weapon->update(controller.direction);
		auto tweak = controller.facing_left() ? -1.f : 1.f;
		sf::Vector2<float> p_pos = {apparent_position.x + weapon->gun_offset.x + tweak, apparent_position.y + sprite_offset.y + weapon->gun_offset.y - collider.dimensions.y / 2.f};
		weapon->set_position(p_pos);
	}
}

void Player::walk() {
	if (animation.state == AnimState::slide) { return; }
	if (animation.state == AnimState::sharp_turn) { collider.physics.acceleration.x *= 0.1f; }
	if (controller.moving_right() && !collider.has_right_collision()) {
		collider.physics.acceleration.x = grounded() ? physics_stats.x_acc * controller.horizontal_movement() : (physics_stats.x_acc / physics_stats.air_multiplier) * controller.horizontal_movement();
	}
	if (controller.moving_left() && !collider.has_left_collision()) {
		collider.physics.acceleration.x = grounded() ? physics_stats.x_acc * controller.horizontal_movement() : (physics_stats.x_acc / physics_stats.air_multiplier) * controller.horizontal_movement();
	}
	if (controller.sprinting()) {
		collider.physics.acceleration.x *= physics_stats.sprint_multiplier;
		if (!tutorial.flags.test(text::TutorialFlags::sprint) && tutorial.current_state == text::TutorialFlags::sprint) {
			cooldowns.sprint_tutorial.update();
			if (cooldowns.sprint_tutorial.is_complete()) {
				tutorial.turn_off();
				tutorial.flags.set(text::TutorialFlags::sprint);
			}
		}
	}
	if (animation.get_frame() == 44 || animation.get_frame() == 46 || animation.get_frame() == 10 || animation.get_frame() == 13) {
		if (animation.animation.keyframe_over() && abs(collider.physics.velocity.x) > 2.5f) { m_services->soundboard.flags.player.set(audio::Player::step); }
	}
}

void Player::hurt(float amount, bool force) {
	if (health.is_dead()) { return; }
	if (!health.invincible() || force) {
		if (shielding() && !force) { return; }
		m_services->ticker.slow_down(25);
		health.inflict(amount, force);
		health_indicator.add(-amount);
		collider.physics.velocity.y = 0.0f;
		collider.physics.acceleration.y = -physics_stats.hurt_acc;
		force_cooldown.start(60);
		collider.spike_trigger = false;
		m_services->soundboard.flags.player.set(audio::Player::hurt);
		hurt_cooldown.start(2);
	}
}

void Player::on_crush(world::Map& map) {
	if (!collider.collision_depths) { return; }
	if (collider.crushed() && alive()) {
		hurt(1024.f, true);
		directions.left_squish.und = collider.horizontal_squish() ? dir::UND::up : dir::UND::neutral;
		directions.left_squish.lr = collider.vertical_squish() ? dir::LR::left : dir::LR::neutral;
		directions.right_squish.und = collider.horizontal_squish() ? dir::UND::down : dir::UND::neutral;
		directions.right_squish.lr = collider.vertical_squish() ? dir::LR::right : dir::LR::neutral;
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.physics.position, collider.dimensions, "player_crush", m_services->styles.colors.nani_white, directions.left_squish));
		map.active_emitters.push_back(vfx::Emitter(*m_services, collider.physics.position, collider.dimensions, "player_crush", m_services->styles.colors.nani_white, directions.right_squish));
		collider.collision_depths = {};
		flags.state.set(State::crushed);
	}
}

void Player::update_antennae() {
	int ctr{0};
	for (auto& a : antennae) {
		if (animation.get_frame() == 44 || animation.get_frame() == 46) {
			antenna_offset.y = -19.f;
		} else if (controller.sprinting()) {
			antenna_offset.y = -13.f;
		} else if (animation.get_frame() == 52) {
			antenna_offset.y = -14.f;
		} else if (animation.get_frame() == 53) {
			antenna_offset.y = -11.f;
		} else if (animation.get_frame() == 79) {
			antenna_offset.y = 2.f;
		} else {
			antenna_offset.y = -17.f;
		}
		if (animation.get_frame() == 57) { antenna_offset.y = -8.f; }
		if (!is_dead()) {
			a.set_target_position(collider.physics.position + antenna_offset);
		} else {
			a.demagnetize(*m_services);
		}
		a.update(*m_services);
		a.collider.sync_components();
		if (controller.facing_right()) {
			antenna_offset.x = ctr % 2 == 0 ? 20.0f : 7.f;
		} else {
			antenna_offset.x = ctr % 2 == 0 ? 2.f : 13.f;
		}
		if (animation.get_frame() == 82) { antenna_offset.x += controller.facing_right() ? 6.f : -6.f; }
		++ctr;
	}
}

void Player::sync_antennae() {
	int ctr{0};
	for (auto& a : antennae) {
		a.set_position(collider.physics.position + antenna_offset);
		a.update(*m_services);
		a.collider.sync_components();
		if (controller.facing_right()) {
			antenna_offset.x = ctr % 2 == 0 ? 18.0f : 7.f;
		} else {
			antenna_offset.x = ctr % 2 == 0 ? 2.f : 13.f;
		}
		++ctr;
	}
}

bool Player::grounded() const { return collider.flags.state.test(shape::State::grounded); }

bool Player::fire_weapon() {
	if (!arsenal || !hotbar) { return false; }
	if (controller.shot() && equipped_weapon().can_shoot()) {
		if (!m_services->soundboard.gun_sounds.contains(equipped_weapon().get_id())) {
			m_services->soundboard.flags.weapon.set(audio::Weapon::bryns_gun);
			flags.state.set(State::impart_recoil);
			return true;
		}
		m_services->soundboard.flags.weapon.set(m_services->soundboard.gun_sounds.at(equipped_weapon().get_id()));
		flags.state.set(State::impart_recoil);
		if (tutorial.current_state == text::TutorialFlags::shoot) {
			tutorial.flags.set(text::TutorialFlags::shoot);
			tutorial.current_state = text::TutorialFlags::map;
			tutorial.turn_off();
		}
		return true;
	}
	return false;
}

void Player::update_invincibility() {
	hurt_cooldown.update();
	if (health.invincible()) {
		flash_sprite();
	} else {
		sprite.setColor(sf::Color::White);
	}
}

void Player::start_over() {
	health.reset();
	controller.unrestrict();
	health.invincibility.start(8);
	flags.state.reset(State::killed);
	animation.triggers.reset(AnimTriggers::end_death);
	animation.post_death.cancel();
	collider.collision_depths = util::CollisionDepth();
	for (auto& a : antennae) {
		a.collider.physics.set_global_friction(physics_stats.antenna_friction);
		a.collider.physics.gravity = 0.f;
	}
	sync_antennae();
	catalog.categories.wardrobe.set_palette(m_services->assets.t_palette_nani);
	catalog.categories.wardrobe.update(texture_updater);
}

void Player::give_drop(item::DropType type, float value) {
	if (type == item::DropType::heart) {
		health.heal(value);
		health_indicator.add(value);
	}
	if (type == item::DropType::orb) {
		player_stats.orbs += static_cast<int>(value);
		orb_indicator.add(value);
		m_services->stats.treasure.total_orbs_collected.update(static_cast<int>(value));
		if (value == 100) { m_services->stats.treasure.blue_orbs.update(); }
		if (orb_indicator.get_amount() > m_services->stats.treasure.highest_indicator_amount.get_count()) { m_services->stats.treasure.highest_indicator_amount.set(static_cast<int>(orb_indicator.get_amount())); }
	}
	if (type == item::DropType::gem) { give_item(static_cast<int>(value) + 97, 1); }
}

void Player::take_item(int item_id, int amount) { catalog.remove_item(*m_services, item_id, amount); }

void Player::equip_item(ApparelType type, int item_id) {
	if (item_id < 1) { return; }
	catalog.equip_item(*m_services, type, item_id);
	catalog.categories.inventory.get_item(item_id).toggle_equip();
}

void Player::unequip_item(ApparelType type, int item_id) {
	if (item_id < 1) { return; }
	catalog.unequip_item(type);
	catalog.categories.inventory.get_item(item_id).toggle_equip();
}

void Player::add_to_hotbar(int id) {
	if (hotbar) {
		hotbar.value().add(id);
	} else {
		hotbar = arms::Hotbar(1);
		hotbar.value().add(id);
	}
}

void Player::remove_from_hotbar(int id) {
	if (hotbar) {
		hotbar.value().remove(id);
		if (hotbar.value().size() < 1) { hotbar = {}; }
	}
}

void Player::give_item(int item_id, int amount) {
	catalog.add_item(*m_services, item_id, 1);
	if (catalog.categories.inventory.items.size() == 1) {
		tutorial.current_state = text::TutorialFlags::inventory;
		tutorial.trigger();
		tutorial.turn_on();
	}
	if (item_id == 16) {
		tutorial.current_state = text::TutorialFlags::map;
		tutorial.trigger();
		tutorial.turn_on();
	}
	if (item_id == 29) {
		health.increase_max_hp(1.f);
		m_services->soundboard.flags.item.set(audio::Item::health_increase);
	}
}

void Player::reset_flags() { flags = {}; }

void Player::total_reset() {
	start_over();
	collider.physics.zero();
	reset_flags();
	arsenal = {};
	update_antennae();
	flags.state.reset(State::killed);
	animation.state = AnimState::idle;
}

void Player::map_reset() {
	animation.state = AnimState::idle;
	flags.state.reset(State::killed);
	if (arsenal) { arsenal.value().reset(); }
}

arms::Weapon& Player::equipped_weapon() { return arsenal.value().get_weapon_at(hotbar.value().get_id()); }

void Player::push_to_loadout(int id, bool from_save) {
	if (!arsenal) { arsenal = arms::Arsenal(*m_services); }
	if (!hotbar && !from_save) { hotbar = arms::Hotbar(1); }
	if (id == 0) {
		m_services->stats.time_trials.bryns_gun = m_services->ticker.in_game_seconds_passed.count();
		auto bg = util::QuestKey{1, 111, 1};
		m_services->quest.process(*m_services, bg);
		tutorial.flags.set(text::TutorialFlags::inventory); // set this in case the player never opened inventory
		tutorial.current_state = text::TutorialFlags::shoot;
		tutorial.trigger();
		tutorial.turn_on();
	}
	if (id == 10) { m_services->quest.progress(fornani::QuestType::destroyers, 122, 1); }
	arsenal.value().push_to_loadout(id);
	if (!from_save) { hotbar.value().add(id); }
	m_services->stats.player.guns_collected.update();
}

void Player::pop_from_loadout(int id) {
	if (!arsenal) { throw std::runtime_error("Cannot pop weapon from empty Arsenal."); }
	if (!hotbar) { throw std::runtime_error("Cannot pop weapon from empty Hotbar."); }
	arsenal.value().pop_from_loadout(id);
	hotbar.value().remove(id);
	if (arsenal.value().empty()) {
		arsenal = {};
		hotbar = {};
	}
}

dir::LR Player::entered_from() const { return (collider.physics.position.x < lookup::SPACING * 8) ? dir::LR::right : dir::LR::left; }

std::string Player::print_direction(bool lr) {
	if (lr) {
		if (controller.facing_left()) return "LEFT";
		if (controller.facing_right()) return "RIGHT";
	}
	return "NULL";
}

} // namespace player