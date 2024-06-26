
#include "Player.hpp"
#include "../item/Drop.hpp"
#include "../../gui/Console.hpp"
#include "../../gui/InventoryWindow.hpp"
#include "../../service/ServiceProvider.hpp"

namespace player {

Player::Player() {}

Player::Player(automa::ServiceProvider& svc) : arsenal(svc), m_services(&svc), health_indicator(svc), orb_indicator(svc), controller(svc) {}

void Player::init(automa::ServiceProvider& svc) {

	m_services = &svc;

	svc.data.load_player_params(*this);
	arsenal = arms::Arsenal(svc);
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

	update_sprite();
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

	//player-controlled actions
	arsenal.switch_weapon(*m_services, controller.arms_switch());
	dash();
	jump();
	wallslide();
	update_animation();

	// check keystate
	if (!controller.get_jump().jumpsquatting()) { walk(); }
	if (!controller.moving()) { collider.physics.acceleration.x = 0.0f; }

	// weapon physics
	if (controller.shot() && !arsenal.loadout.empty()) {
		if (controller.direction.und == dir::UND::down) { collider.physics.acceleration.y += -equipped_weapon().attributes.recoil / 80; }
		if (controller.direction.und == dir::UND::up) { collider.physics.acceleration.y += equipped_weapon().attributes.recoil; }
	}

	collider.update(*m_services);
	health.update();
	health_indicator.update(*m_services, collider.physics.position);
	orb_indicator.update(*m_services, collider.physics.position);
	update_invincibility();
	update_weapon();
	catalog.update(*m_services);

	if (catalog.categories.abilities.has_ability(Abilities::dash)) {
		if (!animation.state.test(AnimState::dash) && !controller.dash_requested()) {
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

	// dashing effect
	sprite.setPosition(sprite_position);
	if (svc.ticker.every_x_frames(8) && animation.state.test(AnimState::dash)) { sprite_history.update(sprite); }
	if (svc.ticker.every_x_frames(8) && !animation.state.test(AnimState::dash)) { sprite_history.flush(); }
	drag_sprite(win, campos);

	// get UV coords
	int u = (int)(animation.get_frame() / asset::NANI_SPRITESHEET_HEIGHT) * asset::NANI_SPRITE_WIDTH;
	int v = (int)(animation.get_frame() % asset::NANI_SPRITESHEET_HEIGHT) * asset::NANI_SPRITE_WIDTH;
	sprite.setTextureRect(sf::IntRect({u, v}, {asset::NANI_SPRITE_WIDTH, asset::NANI_SPRITE_WIDTH}));
	sprite.setOrigin(asset::NANI_SPRITE_WIDTH / 2, asset::NANI_SPRITE_WIDTH / 2);
	sprite.setPosition(sprite_position.x - campos.x, sprite_position.y - campos.y);
	
	if (!arsenal.loadout.empty()) {
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

	if (!arsenal.loadout.empty()) {
		equipped_weapon().sp_gun.setTexture(svc.assets.weapon_textures.at(equipped_weapon().label));
		if (flags.state.test(State::show_weapon)) { equipped_weapon().render(svc, win, campos); }
	}

	if (controller.get_shield().is_shielding()) { controller.get_shield().render(*m_services, win, campos); }

	// texture updater debug
	// texture_updater.debug_render(win, campos);
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
		if (controller.inspecting()) { animation.state.set(AnimState::inspect); }
		if (!(animation.state.test(AnimState::jumpsquat) || animation.state.test(AnimState::land) || animation.state.test(AnimState::rise))) {
			if (controller.inspecting()) { animation.state.set(AnimState::inspect); }
			if (controller.nothing_pressed() && !controller.dashing() && !animation.state.test(AnimState::inspect)) { animation.state.set(AnimState::idle); }
			if (controller.moving() && !controller.dashing() && !controller.sprinting()) { animation.state.set(AnimState::run); }
			if (controller.moving() && controller.sprinting() && !controller.dashing() && !animation.state.test(AnimState::sharp_turn)) { animation.state.set(AnimState::sprint); }
			if (abs(collider.physics.velocity.x) > thresholds.stop && !controller.moving()) { animation.state.set(AnimState::stop); }
			if (quick_direction_switch()) {
				animation.state = {};
				animation.state.set(AnimState::sharp_turn);
			}
		}
	} else {
		if (collider.physics.velocity.y > -thresholds.suspend && collider.physics.velocity.y < thresholds.suspend) { animation.state.set(AnimState::suspend); }
	}

	if (collider.flags.state.test(shape::State::just_landed)) { animation.state.set(AnimState::land); }
	if (animation.state.test(AnimState::fall) && grounded()) { animation.state.set(AnimState::land); }
	if (animation.state.test(AnimState::suspend) && grounded()) { animation.state.set(AnimState::land); }

	if (collider.physics.velocity.y > thresholds.suspend && !grounded()) { animation.state.set(AnimState::fall); }
	if (collider.physics.velocity.y < -thresholds.suspend) { animation.state.set(AnimState::rise); }

	if (catalog.categories.abilities.has_ability(Abilities::dash)) {
		if (controller.dashing() && controller.can_dash()) { animation.state.set(AnimState::dash); }
		if (controller.dash_requested()) {
			animation.state.set(AnimState::dash);
			flags.state.reset(State::show_weapon);
		}
	}
	if (catalog.categories.abilities.has_ability(Abilities::wall_slide)) {
		if (controller.get_wallslide().is_wallsliding()) {
			animation.state = {};
			animation.state.set(AnimState::wallslide);
		}
	}
	if (catalog.categories.abilities.has_ability(Abilities::shield)) {
		if(controller.get_shield().is_shielding() && grounded()) {
			animation.state = {};
			animation.state.set(AnimState::shield);
			controller.prevent_movement();
			flags.state.reset(State::show_weapon);
		}
	}

	if (animation.state.test(AnimState::sit)) { flags.state.reset(State::show_weapon); }

	if (hurt_cooldown.running()) {
		animation.state = {};
		animation.state.set(AnimState::hurt);
	}

	animation.update();
}

void Player::update_sprite() {
	// flip the sprite based on the player's direction
	sf::Vector2<float> right_scale = {1.0f, 1.0f};
	sf::Vector2<float> left_scale = {-1.0f, 1.0f};
	if (controller.facing_left() && sprite.getScale() == right_scale) {
		sprite.scale(-1.0f, 1.0f);
		if (grounded()) { animation.state.set(AnimState::turn); }
	}
	if (controller.facing_right() && sprite.getScale() == left_scale) {
		sprite.scale(-1.0f, 1.0f);
		if (grounded()) { animation.state.set(AnimState::turn); }
	}

	// check for quick turn
	flags.state.reset(State::dir_switch);
	if (controller.quick_turn()) { flags.state.set(State::dir_switch); }

	sprite.setTexture(texture_updater.get_dynamic_texture());
}

void Player::update_transponder(gui::Console& console, gui::InventoryWindow& inventory_window) {
	if (inventory_window.active()) {
		controller.prevent_movement();
		if (controller.transponder_up()) {
			inventory_window.selector.go_up();
		}
		if (controller.transponder_down()) {
			inventory_window.selector.go_down();
		}
		if (controller.transponder_left()) {
			inventory_window.selector.go_left();
		}
		if (controller.transponder_right()) {
			inventory_window.selector.go_right();
		}
		transponder.update(*m_services, inventory_window);
	}
	if (console.active()) {
		controller.prevent_movement();
		if (controller.transponder_skip()) { transponder.skip_ahead(); }
		if (controller.transponder_skip_released()) { transponder.enable_skip(); }
		if (controller.transponder_next()) { transponder.next(); }
		if (controller.transponder_exit()) { transponder.exit(); }
		if (controller.transponder_up()) { transponder.go_up(); }
		if (controller.transponder_down()) { transponder.go_down(); }
		if (controller.transponder_select()) { transponder.select(); }
		transponder.update(*m_services, console);
	}
	transponder.end();
	if (transponder.get_item_shipment() > 0) {
		std::cout << transponder.get_item_shipment() << "\n";
		give_item(transponder.get_item_shipment(), 1);
	}
	if (transponder.get_quest_shipment() > 0) { std::cout << transponder.get_quest_shipment() << "\n"; } // push item to inventory!
}

void Player::flash_sprite() {
	if ((health.invincibility.get_cooldown() / 30) % 2 == 0) {
		sprite.setColor(flcolor::red);
	} else {
		sprite.setColor(flcolor::blue);
	}
}

void Player::drag_sprite(sf::RenderWindow& win, sf::Vector2<float>& campos) {
	auto a{100};
	auto ctr{0};
	for (auto& sp : sprite_history.sprites) {
		sp.setColor(sf::Color(255, 255, 255, a));
		sp.setPosition(sprite_history.positions.at(ctr) - campos);
		if (!m_services->greyblock_mode()) {
			win.draw(sp);
		}
		a += 20;
		++ctr;
	}
}

void Player::calculate_sprite_offset() {
	sprite_offset.y = 0.f;
	if (collider.flags.state.test(shape::State::on_ramp)) { sprite_offset.y = -2.f; }
	sprite_position = {collider.physics.position.x + 9.f, collider.physics.position.y + sprite_offset.y + 1};
	apparent_position = collider.physics.position + collider.dimensions / 2.f;
}

void Player::jump() {
	if (controller.get_jump().began()) {
		collider.flags.movement.set(shape::Movement::jumping);
	} else {
		collider.flags.movement.reset(shape::Movement::jumping);
	}
	if (controller.get_jump().jumpsquat_trigger()) {
		animation.state.set(AnimState::jumpsquat);
		controller.get_jump().start_jumpsquat();
		controller.get_jump().reset_jumpsquat_trigger();
		collider.flags.movement.set(shape::Movement::jumping);
	}
	if (controller.get_jump().jumpsquatting() && !animation.state.test(AnimState::jumpsquat)) {
		controller.get_jump().stop_jumpsquatting();
		controller.get_jump().start();
		collider.physics.acceleration.y = -physics_stats.jump_velocity;
		animation.state.set(AnimState::rise);
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
	if (animation.state.test(AnimState::dash) || controller.dash_requested()) {
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
	if (!catalog.categories.abilities.has_ability(Abilities::wall_slide)) { return; }
	if (controller.get_shield().is_shielding()) { controller.get_shield().sensor.bounds.setPosition(collider.physics.position); }
}

void Player::set_position(sf::Vector2<float> new_pos, bool centered) {
	sf::Vector2<float> offset{};
	offset.x = centered ? collider.dimensions.x * 0.5f : 0.f;
	collider.physics.position = new_pos - offset;
	collider.sync_components();
	update_direction();
	sync_antennae();
}

void Player::update_direction() {
	if (controller.facing_left()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2 - ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	} else if (controller.facing_right()) {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2 + ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	} else {
		anchor_point = {collider.physics.position.x + collider.bounding_box.dimensions.x / 2, collider.physics.position.y + collider.bounding_box.dimensions.y / 2};
	}

	//set directions for grappling hook
	equipped_weapon().projectile.hook.probe_direction = controller.direction;
}

void Player::update_weapon() {
	if (arsenal.armory.empty() || arsenal.loadout.empty()) { return; }
	// clamp extant projectile instances to the weapon's rate
	assert(arsenal.extant_projectile_instances.size() >= arsenal.armory.size());
	for (std::size_t index = 0; index < arsenal.extant_projectile_instances.size(); ++index) {
		auto& count = arsenal.extant_projectile_instances.at(index);
		if (arsenal.armory.at(index)->attributes.rate < 0) { continue; }
		count = std::clamp(count, 0, arsenal.armory.at(index)->attributes.rate);
	}
	// update all weapons in loadout to avoid unusual behavior upon fast weapon switching
	for (auto& weapon : arsenal.loadout) {
		weapon->active_projectiles = extant_instances(weapon->get_id());
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
	if (animation.state.test(AnimState::sharp_turn)) {
		collider.physics.acceleration.x = 0.0f;
		return;
	}
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

void Player::hurt(int amount = 1) {
	if (!health.invincible()) {
		health.inflict(amount);
		health_indicator.add(-amount);
		collider.physics.velocity.y = 0.0f;
		collider.physics.acceleration.y = -physics_stats.hurt_acc;
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
		++extant_instances(equipped_weapon().get_id());
		m_services->soundboard.flags.weapon.set(m_services->soundboard.gun_sounds.at(equipped_weapon().label));
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

void Player::kill() { flags.state.reset(State::alive); }

void Player::start_over() {
	health.reset();
	flags.state.set(State::alive);
}

void Player::give_drop(item::DropType type, int value) {
	if (type == item::DropType::heart) {
		health.heal(value);
		health_indicator.add(value);
	}
	if (type == item::DropType::orb) {
		player_stats.orbs += value;
		orb_indicator.add(value);
	}
}

void Player::give_item(int item_id, int amount) { catalog.add_item(*m_services, item_id, 1); }

void Player::reset_flags() { flags = {}; }

void Player::total_reset() {
	start_over();
	collider.physics.zero();
	reset_flags();
	arsenal.loadout.clear();
	update_antennae();
}

void Player::map_reset() {
	arsenal.extant_projectile_instances = {};
	if (animation.state.test(player::AnimState::inspect)) { animation.state.set(player::AnimState::idle); }
}

arms::Weapon& Player::equipped_weapon() {
	return arsenal.get_current_weapon();
}

int& Player::extant_instances(int index) { return arsenal.extant_projectile_instances.at(index); }

dir::LR Player::entered_from() const { return (collider.physics.position.x < lookup::SPACING * 8) ? dir::LR::right : dir::LR::left; }

std::string Player::print_direction(bool lr) {
	if (lr) {
		if (controller.facing_left()) return "LEFT";
		if (controller.facing_right()) return "RIGHT";
	}
}

} // namespace player