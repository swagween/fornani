
#include <fornani/entities/enemy/catalog/Sentinel.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Sentinel::Sentinel(automa::ServiceProvider& svc, world::Map& map, int variant)
	: Enemy(svc, map, "sentinel"), m_variant{static_cast<SentinelVariant>(variant)}, m_services(&svc), m_map(&map), m_sword{svc.assets.get_texture("sentinel_sword"), 4.0f, 0.85f, {0.f, 0.f}} {
	p_animations = {{"idle", {0, 6, 40, -1}},  {"run", {6, 4, 48, 2}},	 {"turn", {10, 3, 30, 0}},			{"jump", {13, 4, 48, -1}},	 {"land", {19, 3, 48, 0}},		   {"slash", {13, 4, 48, -1}},
					{"swipe", {19, 3, 48, 0}}, {"dash", {25, 4, 20, 1}}, {"prepare_dash", {24, 1, 120, 0}}, {"summon", {29, 1, 180, 0}}, {"charge_swipe", {17, 2, 80, 0}}, {"charge_slash", {22, 2, 80, 0}}};
	animation.set_params(get_params("idle"));
	get_collider().physics.set_friction_componentwise({0.97f, 0.999f});
	get_collider().physics.air_friction = {0.999f, 0.999f};
	get_secondary_collider().set_dimensions({28.f, 28.f});
	get_secondary_collider().set_attribute(shape::ColliderAttributes::no_collision);
	if (m_variant == SentinelVariant::knight) {
		m_shield.emplace(entity::FloatingPart(svc.assets.get_texture("sentinel_shield"), 2.0f, 0.65f, {-86.f, 0.f}));
		m_shield->set_team(arms::Team::guardian);
	}
	flags.general.set(GeneralFlags::has_invincible_channel);
	flags.state.set(StateFlags::no_slowdown);
}

void Sentinel::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { m_services->soundboard.play_sound("tank_death", get_collider().get_center()); }
	flags.state.set(StateFlags::vulnerable);
	flags.general.reset(GeneralFlags::hurt_on_contact);
	flags.general.set(GeneralFlags::player_collision);
	if (is_state(SentinelState::dash)) {
		flags.general.reset(GeneralFlags::player_collision);
		flags.state.reset(StateFlags::vulnerable);
		flags.general.set(GeneralFlags::hurt_on_contact);
	}
	if (is_state(SentinelState::summon) || is_state(SentinelState::prepare_dash)) { flags.state.reset(StateFlags::vulnerable); }
	Enemy::update(svc, map, player);
	if (died()) { return; }

	m_cooldowns.post_attack.update();
	m_cooldowns.post_jump.update();

	hurt_effect.update();
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) { svc.soundboard.play_sound("hit_low"); }
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
	}

	// parts
	auto tweak = is_state(SentinelState::charge_slash) ? sf::Vector2f{-100.f * directions.actual.as_float(), 0.f} : sf::Vector2f{};
	if (is_state(SentinelState::charge_swipe)) { tweak = sf::Vector2f{-50.f * directions.actual.as_float(), -100.f}; }
	// if (is_state(SentinelState::slash)) { tweak = sf::Vector2f{50.f * directions.actual.as_float(), -140.f}; }
	auto sword_offset = sf::Vector2f{-28.f * directions.actual.as_float(), 6.f} + tweak;
	m_sword.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center() + sword_offset);
	if (m_shield) {
		m_shield->update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center());
		m_shield->set_shield({24.f, 100.f}, {0.f, -24.f});
	}
	m_sword.sprite->setOrigin(m_sword.sprite->getLocalBounds().getCenter());
	m_sword.sprite->setTextureRect(sf::IntRect{{0, 0}, {84, 43}});
	m_sword.sprite->setRotation(sf::degrees(0));
	if (is_state(SentinelState::charge_slash)) { m_sword.sprite->setTextureRect(sf::IntRect{{0, 43}, {84, 43}}); }
	if (is_state(SentinelState::charge_swipe)) {
		m_sword.sprite->setTextureRect(sf::IntRect{{0, 43}, {84, 43}});
		if (directions.actual.left()) { m_sword.sprite->setRotation(sf::degrees(-90)); }
		if (directions.actual.right()) { m_sword.sprite->setRotation(sf::degrees(90)); }
	}
	if (is_state(SentinelState::slash)) {
		m_sword.sprite->setTextureRect(sf::IntRect{{0, 43}, {84, 43}});
		if (directions.actual.left()) { m_sword.sprite->setRotation(sf::degrees(-90)); }
		if (directions.actual.right()) { m_sword.sprite->setRotation(sf::degrees(90)); }
	}
	if (secondary_collider) {
		get_secondary_collider().physics.position = get_collider().physics.position + sf::Vector2f{40.f, -20.f};
		get_secondary_collider().physics.position.x += directions.actual.as_float() * -18.f;
		get_secondary_collider().sync_components();
	}

	// attacks
	for (auto [i, attack] : std::views::enumerate(m_attacks)) {
		auto r = i == 1 ? 100.f : 80.f;
		attack.set_constant_radius(r);
		switch (p_state.actual) {
		case SentinelState::slash:
			r = i == 1 ? 100.f : 80.f;
			attack.set_constant_radius(r);
			switch (i) {
			case 0: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{136.f * directions.actual.as_float(), 10.f}); break;
			case 1: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{96.f * directions.actual.as_float(), -60.f}); break;
			case 2: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{48.f * directions.actual.as_float(), -105.f}); break;
			}
			break;
		case SentinelState::swipe:
			r = i == 1 ? 70.f : 50.f;
			attack.set_constant_radius(r);
			switch (i) {
			case 0: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{176.f * directions.actual.as_float(), 54.f}); break;
			case 1: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{106.f * directions.actual.as_float(), -20.f}); break;
			case 2: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{6.f * directions.actual.as_float(), -90.f}); break;
			}
			break;
		}
		if ((animation.get_frame() == 19 && is_state(SentinelState::swipe)) || (animation.get_frame() == 13 && is_state(SentinelState::slash) && animation.is_first_loop())) {
			attack.hit.activate();
		} else {
			attack.hit.deactivate();
		}
		if (attack.hurt_player(player, 2.f, {directions.actual.as_float() * 20.f, -0.4f})) { player.controller.flush_ability(); }
	}

	// behavior
	if (svc.ticker.every_second() && random::percent_chance(10)) { request(SentinelState::run); }
	if (is_hostile() && !m_cooldowns.post_attack.running()) { random::coin_flip() ? request(SentinelState::charge_slash) : request(SentinelState::charge_swipe); }
	if (is_alert()) { request(SentinelState::jump); }
	if (directions.actual.lnr != directions.desired.lnr) { request(SentinelState::turn); }

	state_function = state_function();
}

void Sentinel::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
	if (is_state(SentinelState::slash) || is_state(SentinelState::swipe) || is_state(SentinelState::dash) || is_state(SentinelState::prepare_dash) || is_state(SentinelState::summon)) {
	} else {
		m_sword.render(svc, win, cam);
	}
	if (m_shield && !is_state(SentinelState::dash) && !is_state(SentinelState::prepare_dash) && !is_state(SentinelState::summon)) { m_shield->render(svc, win, cam); }
	// if (svc.greyblock_mode()) {
	for (auto [i, attack] : std::views::enumerate(m_attacks)) {
		// if (attack.hit.active()) { attack.render(win, cam); }
	}
	//}
}

void Sentinel::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { /*debug();*/ }

fsm::StateFunction Sentinel::update_idle() {
	p_state.actual = SentinelState::idle;
	if (change_state(SentinelState::charge_slash, get_params("charge_slash"))) { return SENTINEL_BIND(update_charge_slash); }
	if (change_state(SentinelState::charge_swipe, get_params("charge_swipe"))) { return SENTINEL_BIND(update_charge_swipe); }
	if (change_state(SentinelState::turn, get_params("turn"))) { return SENTINEL_BIND(update_turn); }
	if (change_state(SentinelState::run, get_params("run"))) { return SENTINEL_BIND(update_run); }
	if (change_state(SentinelState::jump, get_params("jump"))) { return SENTINEL_BIND(update_jump); }
	return SENTINEL_BIND(update_idle);
}

fsm::StateFunction Sentinel::update_run() {
	p_state.actual = SentinelState::run;
	if ((animation.get_frame_count() == 1 || animation.get_frame_count() == 3) && animation.keyframe_started()) { m_services->soundboard.play_sound("tank_step", get_collider().get_center()); }
	get_collider().physics.apply_force({attributes.speed * directions.actual.as_float(), 0.f});
	if (animation.is_complete()) {
		if (change_state(SentinelState::turn, get_params("turn"))) { return SENTINEL_BIND(update_turn); }
		request(SentinelState::idle);
		if (change_state(SentinelState::idle, get_params("idle"))) { return SENTINEL_BIND(update_idle); }
	}
	return SENTINEL_BIND(update_run);
}

fsm::StateFunction Sentinel::update_turn() {
	p_state.actual = SentinelState::turn;
	if (animation.complete()) {
		request_flip();
		request(SentinelState::idle);
		if (change_state(SentinelState::idle, get_params("idle"))) { return SENTINEL_BIND(update_idle); }
	}
	return SENTINEL_BIND(update_turn);
}

fsm::StateFunction Sentinel::update_jump() {
	p_state.actual = SentinelState::jump;
	if (animation.just_started()) {
		jump(true);
		m_cooldowns.post_jump.start();
	}
	if (get_collider().grounded() && !m_cooldowns.post_jump.running()) {
		request(SentinelState::land);
		if (change_state(SentinelState::land, get_params("land"))) { return SENTINEL_BIND(update_land); }
	}
	return SENTINEL_BIND(update_jump);
}

fsm::StateFunction Sentinel::update_land() {
	p_state.actual = SentinelState::land;
	if (animation.just_started()) {
		m_services->soundboard.play_sound("thud", get_collider().get_center());
		m_services->camera_controller.shake(10, 0.3f, 200, 20);
	}
	if (animation.complete()) {
		request(SentinelState::prepare_dash);
		if (change_state(SentinelState::prepare_dash, get_params("prepare_dash"))) { return SENTINEL_BIND(update_prepare_dash); }
	}
	return SENTINEL_BIND(update_land);
}

fsm::StateFunction Sentinel::update_swipe() {
	p_state.actual = SentinelState::swipe;
	if (animation.complete()) {
		m_cooldowns.post_attack.start();
		request(SentinelState::idle);
		if (change_state(SentinelState::idle, get_params("idle"))) { return SENTINEL_BIND(update_idle); }
	}
	return SENTINEL_BIND(update_swipe);
}

fsm::StateFunction Sentinel::update_slash() {
	p_state.actual = SentinelState::slash;
	if (animation.just_started()) {
		jump(false);
		m_cooldowns.post_jump.start();
	}
	if (get_collider().grounded() && !m_cooldowns.post_jump.running()) {
		request(SentinelState::land);
		if (change_state(SentinelState::land, get_params("land"))) { return SENTINEL_BIND(update_land); }
	}
	return SENTINEL_BIND(update_slash);
}

fsm::StateFunction Sentinel::update_charge_swipe() {
	p_state.actual = SentinelState::charge_swipe;
	if (animation.just_started()) { m_services->soundboard.play_sound("sentinel_alert", get_collider().get_center()); }
	if (animation.complete()) {
		m_services->soundboard.play_sound("sentinel_swipe_2", get_collider().get_center());
		auto swipe_offset = sf::Vector2f{90.f * directions.actual.as_float(), -50.f};
		m_map->spawn_effect(*m_services, "sword_swipe", get_collider().get_center() + swipe_offset, {}, 0, -directions.actual.as_float());
		get_collider().physics.apply_force({1.0f * directions.actual.as_float(), 0.f});
		request(SentinelState::swipe);
		if (change_state(SentinelState::swipe, get_params("swipe"))) { return SENTINEL_BIND(update_swipe); }
	}
	return SENTINEL_BIND(update_charge_swipe);
}

fsm::StateFunction Sentinel::update_charge_slash() {
	p_state.actual = SentinelState::charge_slash;
	if (animation.just_started()) { m_services->soundboard.play_sound("sentinel_alert", get_collider().get_center()); }
	if (animation.complete()) {
		m_services->soundboard.play_sound("sentinel_swipe_1", get_collider().get_center());
		auto swipe_offset = sf::Vector2f{100.f * directions.actual.as_float(), 0.f};
		m_map->spawn_effect(*m_services, "sword_slash", get_collider().get_center() + swipe_offset, {}, 0, -directions.actual.as_float());
		get_collider().physics.apply_force({1.0f * directions.actual.as_float(), 0.f});
		request(SentinelState::slash);
		if (change_state(SentinelState::slash, get_params("slash"))) { return SENTINEL_BIND(update_slash); }
	}
	return SENTINEL_BIND(update_charge_slash);
}

fsm::StateFunction Sentinel::update_prepare_dash() {
	p_state.actual = SentinelState::prepare_dash;
	if (animation.just_started()) {
		m_services->soundboard.play_sound("sentinel_prepare", get_collider().get_center());
		m_map->spawn_effect(*m_services, "giga_flare", get_collider().get_center());
	}
	shake();
	if (animation.complete()) {
		request(SentinelState::dash);
		if (change_state(SentinelState::dash, get_params("dash"))) { return SENTINEL_BIND(update_dash); }
	}
	return SENTINEL_BIND(update_prepare_dash);
}

fsm::StateFunction Sentinel::update_dash() {
	p_state.actual = SentinelState::dash;
	if (animation.just_started()) { m_services->soundboard.play_sound("sentinel_charge", get_collider().get_center()); }
	get_collider().physics.velocity.x = 20.f * directions.actual.as_float();
	if (animation.complete()) {
		request(SentinelState::summon);
		if (change_state(SentinelState::summon, get_params("summon"))) { return SENTINEL_BIND(update_summon); }
	}
	return SENTINEL_BIND(update_dash);
}

fsm::StateFunction Sentinel::update_summon() {
	p_state.actual = SentinelState::summon;
	shake();
	if (animation.just_started()) { m_services->soundboard.play_sound("sentinel_summon", get_collider().get_center()); }
	if (animation.complete()) {
		for (int i = 0; i < 3; ++i) { m_map->spawn_enemy(18, get_collider().get_center() + random::random_vector_float(-180.f, 180.f) + sf::Vector2f{0.f, -260.f}, 2); }
		if (change_state(SentinelState::turn, get_params("turn"))) { return SENTINEL_BIND(update_turn); }
		request(SentinelState::idle);
		if (change_state(SentinelState::idle, get_params("idle"))) { return SENTINEL_BIND(update_idle); }
	}
	return SENTINEL_BIND(update_summon);
}

void Sentinel::jump(bool forward) {
	auto sign = forward ? directions.actual.as_float() * 6.f : -directions.actual.as_float() * 4.f;
	get_collider().physics.velocity = {sign, -14.f};
}

bool Sentinel::change_state(SentinelState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

void Sentinel::debug() {
	static auto sz = ImVec2{180.f, 450.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Sentinel Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::Text("Animation: %s", animation.label.c_str());
		ImGui::Text("Retreat: %.4f", m_caution.retreat.lengthSquared());
		ImGui::ProgressBar(m_cooldowns.alerted.get_normalized());
		ImGui::Text("Alerted: %i", m_cooldowns.alerted.get());
		ImGui::Separator();
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("dash")) { request(SentinelState::dash); }
		if (ImGui::Button("run")) { request(SentinelState::run); }
		if (ImGui::Button("charge_slash")) { request(SentinelState::charge_slash); }
		if (ImGui::Button("charge_swipe")) { request(SentinelState::charge_swipe); }
		ImGui::End();
	}
}

} // namespace fornani::enemy
