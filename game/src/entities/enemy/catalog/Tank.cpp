
#include "fornani/entities/enemy/catalog/Tank.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Tank::Tank(automa::ServiceProvider& svc, world::Map& map, int variant)
	: Enemy(svc, map, "tank"), m_variant{static_cast<TankVariant>(variant)}, m_weapon(svc, "skycorps_smg"), m_services(&svc), m_map(&map), m_gun{svc.assets.get_texture("tank_gun"), 2.0f, 0.65f, {-12.f, 6.f}}, m_debug{false} {
	m_params = {{"idle", {0, 6, 28, -1}}, {"run", {6, 4, 38, 2}},	{"shoot_horizontal", {10, 4, 22, 0}}, {"shoot_vertical", {14, 4, 22, 0}}, {"jumpsquat", {18, 5, 12, 0, true}}, {"jump", {23, 4, 22, 0, true}},
				{"land", {27, 3, 22, 0}}, {"turn", {30, 2, 32, 0}}, {"type", {32, 2, 128, -1}},			  {"alert", {34, 7, 32, 0}},		  {"pocket", {41, 6, 32, 0}},		   {"sleep", {47, 2, 256, -1}},
				{"drink", {49, 6, 32, 0}}};
	animation.set_params(get_params("type"));
	m_gun.set_magnitude(1.f);
	m_weapon.clip_cooldown_time = 360;
	m_weapon.get().set_team(arms::Team::skycorps);
	get_collider().physics.maximum_velocity = {3.f, 12.f};
	get_collider().physics.air_friction = {0.95f, 0.999f};
	get_secondary_collider().set_dimensions({28.f, 28.f});
	directions.desired.lnr = LNR::left;
	directions.actual.lnr = LNR::left;
	directions.movement.lnr = LNR::neutral;
	m_cooldowns.run.start();
	m_vertical_range.set_dimensions({90.f, 1024.f});
	m_lower_range.set_dimensions({200.f, 32.f});
	m_shoulders.set_dimensions({120.f, 32.f});
}

void Tank::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { m_services->soundboard.play_sound("tank_death", get_collider().get_center()); }
	Enemy::update(svc, map, player);
	if (died()) { return; }

	for (auto& breakable : map.breakables) {
		if (get_collider().jumpbox.overlaps(breakable->get_bounding_box())) { breakable->on_smash(svc, map, 4); }
	}

	flags.state.set(StateFlags::vulnerable); // tank is always vulnerable
	m_weapon.update(svc, map, *this);
	m_weapon.barrel_offset = sf::Vector2f{directions.actual.as_float() * 60.f, 14.f};
	m_caution.avoid_ledges(map, get_collider(), directions.actual, 4);

	auto gun_offset = p_state.actual == TankState::pocket ? sf::Vector2f{directions.actual.as_float() * -30.f, 12.f} : sf::Vector2f{0.f, 4.f};
	m_gun.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center() + gun_offset);

	hurt_effect.update();
	m_cooldowns.post_jump.update();
	is_alert() ? m_cooldowns.alerted.reverse() : m_cooldowns.alerted.update(player.is_sneaking() ? 2 : 1);
	m_cooldowns.post_shoot.update();

	if (m_cooldowns.alerted.is_complete()) { m_mode = TankMode::neutral; }

	// reset animation states to determine next animation state
	directions.movement.lnr = get_collider().physics.velocity.x > 0.f ? LNR::right : LNR::left;
	if (secondary_collider) { get_secondary_collider().set_position(get_collider().physics.position - sf::Vector2f{-26.f, 14.f}); }

	m_vertical_range.set_position(get_collider().bounding_box.get_position() -
								  sf::Vector2f{(m_vertical_range.get_dimensions().x * 0.5f) - (get_collider().dimensions.x * 0.5f), (m_vertical_range.get_dimensions().y) - (get_collider().dimensions.y * 0.5f)});
	m_shoulders.set_position(get_collider().bounding_box.get_position() - sf::Vector2f{(m_shoulders.get_dimensions().x * 0.5f) - (get_collider().dimensions.x * 0.5f), m_shoulders.get_dimensions().y});
	m_lower_range.set_position(get_collider().bounding_box.get_position() - sf::Vector2f{(m_lower_range.get_dimensions().x * 0.5f) - (get_collider().dimensions.x * 0.5f), -(get_collider().dimensions.y - m_lower_range.get_dimensions().y)});

	m_sounds.hurt = random::percent_chance(50) ? audio::Tank::hurt_1 : audio::Tank::hurt_2;
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) { svc.soundboard.flags.tank.set(m_sounds.hurt); }
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
	}

	if (svc.ticker.every_x_ticks(20)) {
		if (random::percent_chance(8) && !m_caution.danger()) { request(TankState::run); }
	}

	// handle ledges
	bool retreating{};
	auto has_clearance = !m_caution.detected_ceiling(map, get_collider(), sf::Vector2f{0.f, 32.f});
	if (get_collider().grounded()) {
		if (m_caution.detected_step(map, get_collider(), directions.actual, sf::Vector2f{-16.f, 32.f}) && (get_collider().physics.is_moving_horizontally(0.01f) || is_mid_run()) && has_clearance) { request(TankState::jumpsquat); }
		if (m_caution.retreat.lengthSquared() > 0.f) {
			set_flag(TankFlags::shorthop);
			request(TankState::jumpsquat);
			retreating = true;
		}
	} else {
		request(TankState::jump);
	}

	if (!retreating) {
		if (is_alert() && !player.is_sneaking()) {
			auto shoot_type = player.get_collider().bounding_box.overlaps(m_vertical_range) ? TankState::shoot_vertical : TankState::shoot_horizontal;
			if (has_clearance) {
				random::percent_chance(50.f) && !m_cooldowns.post_shoot.running() ? request(shoot_type) : !m_cooldowns.post_jump.running() ? request(TankState::jumpsquat) : request(TankState::run);
			} else if (!m_cooldowns.post_shoot.running()) {
				request(shoot_type);
			}
			// player is close
			if (player.get_collider().bounding_box.overlaps(m_shoulders) && random::percent_chance(20.f)) {
				request(TankState::jumpsquat);
				set_flag(TankFlags::shorthop);
			}
			if (player.get_collider().bounding_box.overlaps(m_lower_range) && random::percent_chance(40.f) && !m_cooldowns.post_jump.running()) { request(TankState::jumpsquat); }

			// just alerted
			if (m_mode == TankMode::neutral) { request(TankState::alert); }
		}

		auto projectile_detected = m_caution.projectile_detected(map, physical.hostile_range, arms::Team::skycorps);
		if (projectile_detected.lnr != LNR::neutral && m_mode == TankMode::neutral) {
			// incoming bullet
			request(TankState::alert);
		} else if (m_mode == TankMode::neutral && !is_alert()) {
			// try to go find a seat
			if (seek_home(map)) { m_variant == TankVariant::typist ? request(TankState::type) : request(TankState::sleep); }
			if (flags.state.test(StateFlags::advance)) { request(TankState::run); }
			if (has_flag_set(TankFlags::show_weapon)) { request(TankState::pocket); }
		} else {
			// face_player(player);
		}
		if (directions.actual.lnr != directions.desired.lnr && !is_state(TankState::type)) { request(TankState::turn); }
	}

	if (just_died()) { m_services->soundboard.play_sound("tank_death", get_collider().get_center()); }

	state_function = state_function();
}

void Tank::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
	if (p_state.actual != TankState::shoot_vertical) { m_gun.sprite->setRotation(sf::degrees(0)); }
	if (has_flag_set(TankFlags::show_weapon)) { m_gun.render(svc, win, cam); }
	if (svc.greyblock_mode()) {
		m_vertical_range.render(win, cam);
		m_caution.debug_render(win, cam);
		m_shoulders.render(win, cam, sf::Color{230, 0, 20, 80});
		m_lower_range.render(win, cam, sf::Color{130, 0, 230, 80});
		auto box = sf::RectangleShape{};
		box.setPosition(get_collider().physics.position - cam);
		m_mode == TankMode::hostile ? box.setFillColor(colors::red) : box.setFillColor(colors::green);
		box.setSize({10.f, 10.f});
		win.draw(box);
	}
}

void Tank::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_debug) { debug(); }
}

fsm::StateFunction Tank::update_idle() {
	animation.label = "idle";
	p_state.actual = TankState::idle;
	if (change_state(TankState::pocket, get_params("pocket"))) { return TANK_BIND(update_pocket); }
	if (change_state(TankState::alert, get_params("alert"))) { return TANK_BIND(update_alert); }
	if (change_state(TankState::turn, get_params("turn"))) { return TANK_BIND(update_turn); }
	if (change_state(TankState::run, get_params("run"))) { return TANK_BIND(update_run); }
	if (change_state(TankState::pocket, get_params("pocket"))) { return TANK_BIND(update_pocket); }
	if (change_state(TankState::jumpsquat, get_params("jumpsquat"))) { return TANK_BIND(update_jumpsquat); }
	if (change_state(TankState::shoot_horizontal, get_params("shoot_horizontal"))) { return TANK_BIND(update_shoot_horizontal); }
	if (change_state(TankState::shoot_vertical, get_params("shoot_vertical"))) { return TANK_BIND(update_shoot_vertical); }
	if (change_state(TankState::type, get_params("type"))) { return TANK_BIND(update_type); }
	if (change_state(TankState::sleep, get_params("sleep"))) { return TANK_BIND(update_sleep); }
	return TANK_BIND(update_idle);
}

fsm::StateFunction Tank::update_turn() {
	animation.label = "turn";
	p_state.actual = TankState::turn;
	if (animation.complete()) {
		request_flip();
		if (change_state(TankState::pocket, get_params("pocket"))) { return TANK_BIND(update_pocket); }
		if (change_state(TankState::type, get_params("type"))) { return TANK_BIND(update_type); }
		if (change_state(TankState::sleep, get_params("sleep"))) { return TANK_BIND(update_sleep); }
		request(TankState::idle);
		if (change_state(TankState::idle, get_params("idle"))) { return TANK_BIND(update_idle); }
	}
	return TANK_BIND(update_turn);
}

fsm::StateFunction Tank::update_type() {
	animation.label = "type";
	p_state.actual = TankState::type;
	if (m_services->ticker.every_second()) {
		if (random::percent_chance(10)) { request(TankState::drink); }
		if (change_state(TankState::drink, get_params("drink"))) { return TANK_BIND(update_drink); }
	}
	if (change_state(TankState::alert, get_params("alert"))) { return TANK_BIND(update_alert); }
	if (change_state(TankState::run, get_params("run"))) { return TANK_BIND(update_run); }
	if (change_state(TankState::idle, get_params("idle"))) { return TANK_BIND(update_idle); }
	return TANK_BIND(update_type);
}

fsm::StateFunction Tank::update_run() {
	animation.label = "run";
	p_state.actual = TankState::run;
	if ((animation.get_frame_count() == 1 || animation.get_frame_count() == 3) && animation.keyframe_started()) { m_services->soundboard.play_sound("tank_step", get_collider().get_center()); }
	get_collider().physics.apply_force({attributes.speed * directions.actual.as_float(), 0.f});
	m_cooldowns.run.update();
	if (change_state(TankState::pocket, get_params("pocket"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_pocket);
	}
	if (change_state(TankState::alert, get_params("alert"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_alert);
	}
	if (change_state(TankState::jump, get_params("jump"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_jump);
	}
	if (change_state(TankState::turn, get_params("turn"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_turn);
	}
	if (change_state(TankState::jumpsquat, get_params("jumpsquat"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_jumpsquat);
	}
	if (change_state(TankState::alert, get_params("alert"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_alert);
	}
	if (change_state(TankState::type, get_params("type"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_type);
	}
	if (change_state(TankState::sleep, get_params("sleep"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_sleep);
	}
	if (m_caution.danger() || animation.complete()) {
		m_cooldowns.run.start();
		request(TankState::idle);
		if (change_state(TankState::idle, get_params("idle"))) { return TANK_BIND(update_idle); }
	}
	if (change_state(TankState::shoot_horizontal, get_params("shoot_horizontal"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_shoot_horizontal);
	}
	if (change_state(TankState::shoot_vertical, get_params("shoot_vertical"))) {
		m_cooldowns.run.start();
		return TANK_BIND(update_shoot_vertical);
	}
	return TANK_BIND(update_run);
}

fsm::StateFunction Tank::update_shoot_horizontal() {
	animation.label = "shoot";
	p_state.actual = TankState::shoot_horizontal;
	set_flag(TankFlags::show_weapon);
	if (change_state(TankState::pocket, get_params("pocket"))) { return TANK_BIND(update_pocket); }
	if (change_state(TankState::turn, get_params("turn"))) { return TANK_BIND(update_turn); }
	if (!m_weapon.get().cooling_down() && animation.get_frame_count() == 2) {
		m_weapon.shoot(*m_services, *m_map);
		m_gun.set_position(m_gun.get_position() + directions.actual.get_vector() * -18.f);
	}
	if (animation.complete() && animation.keyframe_over()) {
		m_cooldowns.post_shoot.start();
		m_weapon.clip_cooldown.start(m_weapon.clip_cooldown_time);
		request(TankState::idle);
		if (change_state(TankState::idle, get_params("idle"))) { return TANK_BIND(update_idle); }
	}
	return TANK_BIND(update_shoot_horizontal);
}

fsm::StateFunction Tank::update_jumpsquat() {
	animation.label = "jumpsquat";
	p_state.actual = TankState::jumpsquat;
	set_flag(TankFlags::show_weapon);
	if (animation.complete()) {
		request(TankState::jump);
		if (change_state(TankState::jump, get_params("jump"))) {
			impulse.start(3);
			m_services->soundboard.play_sound("jump_low", get_collider().get_center());
			return TANK_BIND(update_jump);
		}
	}
	return TANK_BIND(update_jumpsquat);
}

fsm::StateFunction Tank::update_jump() {
	animation.label = "jump";
	p_state.actual = TankState::jump;
	if (has_flag_set(TankFlags::shorthop)) {
		get_collider().physics.apply_force({-attributes.speed * directions.actual.as_float(), 0.f});
		if (impulse.running()) { get_collider().physics.apply_force({0, -40.f}); }
		if (!m_weapon.get().cooling_down()) {
			m_weapon.shoot(*m_services, *m_map);
			m_gun.set_position(m_gun.get_position() + directions.actual.get_vector() * -18.f);
		}
	} else {
		get_collider().physics.apply_force({attributes.speed * directions.actual.as_float(), 0.f});
		if (impulse.running()) { get_collider().physics.apply_force({0, -100.f}); }
	}
	if (animation.complete() && get_collider().grounded()) {
		set_flag(TankFlags::shorthop, false);
		request(TankState::land);
		m_services->soundboard.play_sound("thud", get_collider().get_center());
		m_services->camera_controller.shake(10, 0.3f, 200, 20);
		if (change_state(TankState::land, get_params("land"))) { return TANK_BIND(update_land); }
	}
	return TANK_BIND(update_jump);
}

fsm::StateFunction Tank::update_land() {
	animation.label = "land";
	p_state.actual = TankState::land;
	if (animation.complete()) {
		m_cooldowns.post_jump.start();
		if (change_state(TankState::pocket, get_params("pocket"))) { return TANK_BIND(update_pocket); }
		if (change_state(TankState::shoot_horizontal, get_params("shoot_horizontal"))) { return TANK_BIND(update_shoot_horizontal); }
		if (change_state(TankState::shoot_vertical, get_params("shoot_vertical"))) { return TANK_BIND(update_shoot_vertical); }
		if (change_state(TankState::turn, get_params("turn"))) { return TANK_BIND(update_turn); }
		request(TankState::idle);
		if (change_state(TankState::idle, get_params("idle"))) { return TANK_BIND(update_idle); }
	}
	return TANK_BIND(update_land);
}

fsm::StateFunction Tank::update_shoot_vertical() {
	animation.label = "shoot_vertical";
	p_state.actual = TankState::shoot_vertical;
	set_flag(TankFlags::show_weapon);
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
		m_cooldowns.post_shoot.start();
		m_weapon.clip_cooldown.start(m_weapon.clip_cooldown_time);
		if (change_state(TankState::pocket, get_params("pocket"))) { return TANK_BIND(update_pocket); }
		if (change_state(TankState::turn, get_params("turn"))) { return TANK_BIND(update_turn); }
		if (change_state(TankState::run, get_params("run"))) { return TANK_BIND(update_run); }
		if (change_state(TankState::shoot_vertical, get_params("shoot_vertical"))) { return TANK_BIND(update_shoot_vertical); }
		request(TankState::idle);
		if (change_state(TankState::idle, get_params("idle"))) { return TANK_BIND(update_idle); }
	}
	return TANK_BIND(update_shoot_vertical);
}

fsm::StateFunction Tank::update_alert() {
	animation.label = "alert";
	p_state.actual = TankState::alert;
	if (animation.just_started()) {
		m_cooldowns.alerted.start();
		if (random::percent_chance(50)) {
			m_services->soundboard.play_sound("tank_alert_1", get_collider().get_center());
		} else {
			m_services->soundboard.play_sound("tank_alert_2", get_collider().get_center());
		}
	}
	if (animation.complete()) {
		m_mode = TankMode::hostile;
		request(TankState::pocket);
		if (change_state(TankState::pocket, get_params("pocket"))) { return TANK_BIND(update_pocket); }
	}
	return TANK_BIND(update_alert);
}

fsm::StateFunction Tank::update_pocket() {
	animation.label = "pocket";
	p_state.actual = TankState::pocket;
	if (animation.complete()) {
		toggle_flag(TankFlags::show_weapon);
		if (change_state(TankState::turn, get_params("turn"))) { return TANK_BIND(update_turn); }
		if (change_state(TankState::shoot_vertical, get_params("shoot_vertical"))) { return TANK_BIND(update_shoot_vertical); }
		if (change_state(TankState::shoot_horizontal, get_params("shoot_horizontal"))) { return TANK_BIND(update_shoot_horizontal); }
		request(TankState::idle);
		if (change_state(TankState::idle, get_params("idle"))) { return TANK_BIND(update_idle); }
	}
	return TANK_BIND(update_pocket);
}

fsm::StateFunction Tank::update_sleep() {
	animation.label = "sleep";
	p_state.actual = TankState::sleep;
	if (hostility_triggered()) { request(TankState::alert); }
	if (is_hurt()) { request(TankState::alert); }
	if (ccm::abs(get_collider().physics.actual_velocity().x) > 0.01f) { request(TankState::idle); }
	if (change_state(TankState::alert, get_params("alert"))) { return TANK_BIND(update_alert); }
	if (change_state(TankState::idle, get_params("idle"))) { return TANK_BIND(update_idle); }
	return TANK_BIND(update_sleep);
}

fsm::StateFunction Tank::update_drink() {
	animation.label = "drink";
	p_state.actual = TankState::drink;
	if (animation.just_started()) { m_services->soundboard.play_sound("tank_sip", get_collider().get_center()); }
	if (change_state(TankState::alert, get_params("alert"))) { return TANK_BIND(update_alert); }
	if (animation.complete()) {
		request(TankState::type);
		if (change_state(TankState::type, get_params("type"))) { return TANK_BIND(update_type); }
	}
	return TANK_BIND(update_drink);
}

bool Tank::change_state(TankState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

void Tank::debug() {
	static auto sz = ImVec2{180.f, 450.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Tank Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::Text("Animation: %s", animation.label.c_str());
		ImGui::Text("Retreat: %.4f", m_caution.retreat.lengthSquared());
		ImGui::ProgressBar(m_cooldowns.alerted.get_normalized());
		ImGui::Text("Alerted: %i", m_cooldowns.alerted.get());
		ImGui::Separator();
		ImGui::Text("Post Shoot: %i", m_cooldowns.post_shoot.get());
		ImGui::SameLine();
		ImGui::ProgressBar(m_cooldowns.post_shoot.get_normalized(), ImVec2{80.f, 16.f});
		ImGui::Text("Post Jump: %i", m_cooldowns.post_jump.get());
		ImGui::SameLine();
		ImGui::ProgressBar(m_cooldowns.post_jump.get_normalized(), ImVec2{90.f, 16.f});
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("pocket")) { request(TankState::pocket); }
		if (ImGui::Button("jump")) { request(TankState::jumpsquat); }
		if (ImGui::Button("alert")) { request(TankState::alert); }
		if (ImGui::Button("run")) { request(TankState::run); }
		if (ImGui::Button("shoot")) { request(TankState::shoot_horizontal); }
		ImGui::End();
	}
}

} // namespace fornani::enemy
