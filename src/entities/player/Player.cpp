#include "Player.hpp"
#include "../../gui/Console.hpp"
#include "../../gui/InventoryWindow.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../item/Drop.hpp"

namespace player {

Player::Player(automa::ServiceProvider& svc) : arsenal(svc), m_services(&svc), health_indicator(svc), orb_indicator(svc), controller(svc) {}

void Player::init(automa::ServiceProvider& svc) {

	m_services = &svc;

	svc.data.load_player_params(*this);
	health_indicator.init(svc, 0);
	orb_indicator.init(svc, 1);

	health.set_invincibility(400);

	collider = shape::Collider(sf::Vector2<float>{PLAYER_WIDTH, PLAYER_HEIGHT}, sf::Vector2<float>{PLAYER_START_X, PLAYER_START_Y});
	collider.physics = components::PhysicsComponent({physics_stats.ground_fric, physics_stats.ground_fric}, physics_stats.mass);

	collider.physics.set_constant_friction({physics_stats.ground_fric, physics_stats.air_fric});

	anchor_point = {collider.physics.position.x + PLAYER_WIDTH / 2, collider.physics.position.y + PLAYER_HEIGHT / 2};

	antennae.push_back(vfx::Gravitator(collider.physics.position, svc.styles.colors.bright_orange, antenna_force));
	antennae.push_back(vfx::Gravitator(collider.physics.position, svc.styles.colors.bright_orange, antenna_force, {2.f, 4.f}));

	float back_fric{0.93f};

	antennae[0].collider.physics = components::PhysicsComponent(sf::Vector2<float>{back_fric, back_fric}, 1.0f);
	antennae[0].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);
	antennae[1].collider.physics = components::PhysicsComponent(sf::Vector2<float>{back_fric, back_fric}, 1.0f);
	antennae[1].collider.physics.maximum_velocity = sf::Vector2<float>(antenna_speed, antenna_speed);

	sprite_dimensions = {48.f, 48.f};

	// sprites
	sprite.setTexture(svc.assets.t_nani);

	texture_updater.load_base_texture(svc.assets.t_nani);
	texture_updater.load_pixel_map(svc.assets.t_palette_nani);
}

void Player::update(gui::Console& console, gui::InventoryWindow& inventory_window) {

	invincible() ? collider.draw_hurtbox.setFillColor(m_services->styles.colors.red) : collider.draw_hurtbox.setFillColor(m_services->styles.colors.blue);

	collider.flags.general.set(shape::General::complex);
	if (!catalog.categories.abilities.has_ability(Abilities::dash)) { controller.nullify_dash(); }

	collider.physics.gravity = physics_stats.grav;
	collider.physics.maximum_velocity = physics_stats.maximum_velocity;
	collider.physics.ground_friction = {physics_stats.ground_fric, physics_stats.ground_fric};
	collider.physics.air_friction = {physics_stats.air_fric, physics_stats.air_fric};

	update_direction();
	grounded() ? controller.ground() : controller.unground();
	controller.update(*m_services);
	update_transponder(console, inventory_window);

	if (grounded()) { controller.reset_dash_count(); }

	// do this elsehwere later
	if (collider.flags.state.test(shape::State::just_landed)) { m_services->soundboard.flags.player.set(audio::Player::land); }
	collider.flags.state.reset(shape::State::just_landed);

	// player-controlled actions
	if (arsenal) { arsenal.value().switch_weapon(*m_services, static_cast<int>(controller.arms_switch())); }
	dash();
	jump();
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

	for (auto& force : accumulated_forces) { collider.physics.apply_force(force); }
	accumulated_forces.clear();
	collider.physics.position += forced_momentum;
	if (controller.moving() || collider.has_horizontal_collision() || collider.flags.external_state.test(shape::ExternalState::vert_world_collision) || collider.world_grounded()) { forced_momentum = {}; }

	collider.update(*m_services);
	health.update();
	health_indicator.update(*m_services, collider.physics.position);
	orb_indicator.update(*m_services, collider.physics.position);
	update_invincibility();
	update_weapon();
	catalog.update(*m_services);
	if (m_services->ticker.every_x_ticks(10)) { collider.collision_depths = {}; }
	if (collider.crushed()) {
		if (!just_died()) { health_indicator.add(-64); }
		kill();
	}

	if (catalog.categories.abilities.has_ability(Abilities::dash)) {
		if (!(animation.state == AnimState::dash) && !controller.dash_requested()) {
			controller.stop_dashing();
			controller.cancel_dash_request();
			collider.flags.dash.reset(shape::Dash::dash_cancel_collision);
		}
	}

	update_antennae();
}

void Player::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos) {

	sf::Vector2<float> player_pos = apparent_position - campos;
	calculate_sprite_offset();
	force_cooldown.update();

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

	if (arsenal) {
		collider.flags.general.set(shape::General::complex);
		equipped_weapon().sp_gun_back.setTexture(svc.assets.weapon_textures.at(equipped_weapon().label));
		if (flags.state.test(State::show_weapon)) { equipped_weapon().render_back(svc, win, campos); }
	}

	if (flags.state.test(State::alive)) {
		if (svc.greyblock_mode()) {
			collider.render(win, campos);
		} else {
			antennae[1].render(svc, win, campos, 1);
			win.draw(sprite);
			antennae[0].render(svc, win, campos, 1);
		}
	}

	if (arsenal) {
		equipped_weapon().sp_gun.setTexture(svc.assets.weapon_textures.at(equipped_weapon().label));
		if (flags.state.test(State::show_weapon)) { equipped_weapon().render(svc, win, campos); }
	}

	if (controller.get_shield().active() && catalog.categories.abilities.has_ability(Abilities::shield)) { controller.get_shield().render(*m_services, win, campos); }

	collider.flush_positions();

}

void Player::render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (orb_indicator.active()) { health_indicator.shift(); }
	health_indicator.render(svc, win, cam);
	orb_indicator.render(svc, win, cam);
}

void Player::assign_texture(sf::Texture& tex) { sprite.setTexture(tex); }

void Player::update_animation() {

	flags.state.set(State::show_weapon);

	if (grounded()) {
		if (controller.inspecting()) { animation.state = AnimState::inspect; }
		if (!(animation.state == AnimState::jumpsquat || animation.state == AnimState::land || animation.state == AnimState::rise)) {
			if (controller.inspecting()) { animation.state = AnimState::inspect; }
			if (controller.nothing_pressed() && !controller.dashing() && !(animation.state == AnimState::inspect) && !(animation.state == AnimState::sit)) { animation.state = AnimState::idle; }
			if (controller.moving() && !controller.dashing() && !controller.sprinting()) { animation.state = AnimState::run; }
			if (controller.moving() && controller.sprinting() && !controller.dashing() && !(animation.state == AnimState::sharp_turn)) { animation.state = AnimState::sprint; }
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
	if (collider.physics.velocity.y < -thresholds.suspend) { animation.state = AnimState::rise; }

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
	if (catalog.categories.abilities.has_ability(Abilities::shield)) {
		if (controller.get_shield().is_shielding() && grounded() && !(animation.state == AnimState::land) && !(animation.state == AnimState::fall)) {
			animation.state = AnimState::shield;
			controller.prevent_movement();
			flags.state.reset(State::show_weapon);
		}
	}
	if (animation.state == AnimState::sit) { flags.state.reset(State::show_weapon); }
	if (hurt_cooldown.running()) { animation.state = AnimState::hurt; }

	animation.update();
}

void Player::update_sprite() {

	if (animation.triggers.consume(AnimTriggers::flip)) {
		sprite.scale(-1.0f, 1.0f);
		if (animation.animation.label == "turn" || animation.animation.label == "sharp_turn") {
			animation.animation.set_params(idle);
		}
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
	if (controller.facing_left() && sprite.getScale() == right_scale) { animation.state = controller.quick_turn() ? AnimState::sharp_turn : AnimState::turn; }
	if (controller.facing_right() && sprite.getScale() == left_scale) { animation.state = controller.quick_turn() ? AnimState::sharp_turn : AnimState::turn; }
	if (quick_direction_switch()) { animation.state = controller.quick_turn() ? AnimState::sharp_turn : AnimState::turn; }
}

void Player::update_transponder(gui::Console& console, gui::InventoryWindow& inventory_window) {
	if (inventory_window.active()) {
		controller.restrict();
		controller.prevent_movement();
		if (controller.transponder_up()) { inventory_window.selector.go_up(); }
		if (controller.transponder_down()) { inventory_window.selector.go_down(); }
		if (controller.transponder_left()) { inventory_window.selector.go_left(); }
		if (controller.transponder_right()) { inventory_window.selector.go_right(); }
		transponder.update(*m_services, inventory_window);
	}
	if (console.active()) {
		controller.restrict();
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
	if (qs > 0) { /* do something with quest tracker */
		quest_code = util::QuestCode(qs);
		if (quest_code.value().reveal_item()) { catalog.categories.inventory.reveal_item(quest_code.value().get_id()); }
		if (quest_code.value().progress_quest()) { m_services->quest.progress(static_cast<fornani::QuestType>(quest_code.value().get_type()), quest_code.value().get_id()); }
		// handle other quest code types
		quest_code = {};
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
	apparent_position = collider.get_average_tick_position() + collider.dimensions / 2.f;
}

void Player::jump() {
	if (controller.get_jump().began()) {
		collider.flags.movement.set(shape::Movement::jumping);
	} else {
		collider.flags.movement.reset(shape::Movement::jumping);
	}
	if (controller.get_jump().jumpsquat_trigger()) {
		animation.state = AnimState::jumpsquat;
		controller.get_jump().start_jumpsquat();
		controller.get_jump().reset_jumpsquat_trigger();
		collider.flags.movement.set(shape::Movement::jumping);
	}
	if (controller.get_jump().jumpsquatting() && !(animation.state == AnimState::jumpsquat)) {
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
	if (arsenal) { equipped_weapon().projectile.hook.probe_direction = controller.direction; }
}

void Player::update_weapon() {
	if (!arsenal) { return; }
	// update all weapons in loadout to avoid unusual behavior upon fast weapon switching
	for (auto& weapon : arsenal.value().get_loadout()) {
		weapon->firing_direction = controller.direction;
		weapon->update(controller.direction);
		sf::Vector2<float> p_pos = {apparent_position.x + weapon->gun_offset.x, apparent_position.y + sprite_offset.y + weapon->gun_offset.y - collider.dimensions.y / 2.f};
		weapon->set_position(p_pos);
	}
	if (controller.facing_right()) {
		hand_position = {28, 36};
	} else {
		hand_position = {20, 36};
	}
}

void Player::walk() {
	if (animation.state == AnimState::sharp_turn) { collider.physics.acceleration.x *= 0.1f; }
	if (controller.moving_right() && !collider.has_right_collision()) {
		collider.physics.acceleration.x = grounded() ? physics_stats.x_acc * controller.horizontal_movement() : (physics_stats.x_acc / physics_stats.air_multiplier) * controller.horizontal_movement();
	}
	if (controller.moving_left() && !collider.has_left_collision()) {
		collider.physics.acceleration.x = grounded() ? physics_stats.x_acc * controller.horizontal_movement() : (physics_stats.x_acc / physics_stats.air_multiplier) * controller.horizontal_movement();
	}
	if (controller.sprinting()) { collider.physics.acceleration.x *= physics_stats.sprint_multiplier; }
	if (animation.get_frame() == 44 || animation.get_frame() == 46 || animation.get_frame() == 10 || animation.get_frame() == 13) {
		if (animation.animation.keyframe_over() && abs(collider.physics.velocity.x) > 2.5f) { m_services->soundboard.flags.player.set(audio::Player::step); }
	}
}

void Player::hurt(float amount = 1.f) {
	if (!health.invincible()) {
		if (shielding()) { return; }
		m_services->ticker.slow_down(25);
		health.inflict(amount);
		health_indicator.add(-amount);
		collider.physics.velocity.y = 0.0f;
		collider.physics.acceleration.y = -physics_stats.hurt_acc;
		force_cooldown.start(60);
		collider.spike_trigger = false;
		m_services->soundboard.flags.player.set(audio::Player::hurt);
		hurt_cooldown.start(2);
	}
	if (health.is_dead()) { kill(); }
}

void Player::update_antennae() {
	int ctr{0};
	for (auto& a : antennae) {
		if (animation.get_frame() == 44 || animation.get_frame() == 46) {
			antenna_offset.y = -15.f;
		} else if (controller.sprinting()) {
			antenna_offset.y = -9.f;
		} else if (animation.get_frame() == 52) {
			antenna_offset.y = -10.f;
		} else if (animation.get_frame() == 53) {
			antenna_offset.y = -7.f;
		} else {
			antenna_offset.y = -13.f;
		}
		if (animation.get_frame() == 57) { antenna_offset.y = -4.f; }
		a.set_target_position(collider.physics.position + antenna_offset);
		a.update(*m_services);
		a.collider.sync_components();
		if (controller.facing_right()) {
			antenna_offset.x = ctr % 2 == 0 ? 18.0f : 7.f;
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
	if (controller.shot() && equipped_weapon().can_shoot()) {
		if (!m_services->soundboard.gun_sounds.contains(equipped_weapon().label)) {
			m_services->soundboard.flags.weapon.set(audio::Weapon::bryns_gun);
			flags.state.set(State::impart_recoil);
			return true;
		}
		m_services->soundboard.flags.weapon.set(m_services->soundboard.gun_sounds.at(equipped_weapon().label));
		flags.state.set(State::impart_recoil);
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

void Player::kill() {
	flags.state.reset(State::alive);
	flags.state.set(State::killed);
}

void Player::start_over() {
	health.reset();
	flags.state.set(State::alive);
	collider.collision_depths = {};
}

void Player::give_drop(item::DropType type, float value) {
	if (type == item::DropType::heart) {
		health.heal(value);
		health_indicator.add(value);
	}
	if (type == item::DropType::orb) {
		player_stats.orbs += static_cast<int>(value);
		orb_indicator.add(value);
	}
}

void Player::give_item(int item_id, int amount) { catalog.add_item(*m_services, item_id, 1); }

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
	if (animation.state == AnimState::inspect) { animation.state = AnimState::idle; }
	if (flags.state.test(State::killed)) { animation.state = AnimState::idle; }
	flags.state.reset(State::killed);
}

arms::Weapon& Player::equipped_weapon() { return arsenal.value().get_current_weapon(); }

void Player::push_to_loadout(int id) {
	if (!arsenal) { arsenal = arms::Arsenal(*m_services); }
	arsenal.value().push_to_loadout(id);
}

void Player::pop_from_loadout(int id) {
	if (!arsenal) { throw std::runtime_error("Cannot pop weapon from empty Arsenal."); }
	arsenal.value().pop_from_loadout(id);
	if (arsenal.value().empty()) { arsenal = {}; }
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