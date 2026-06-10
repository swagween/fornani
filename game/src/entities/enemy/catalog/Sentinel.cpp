
#include <fornani/entities/enemy/catalog/Sentinel.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Sentinel::Sentinel(automa::ServiceProvider& svc, world::Map& map, int variant)
	: Enemy(svc, map, "sentinel"), m_variant{static_cast<SentinelVariant>(variant)}, m_services(&svc), m_map(&map), m_sword{svc.assets.get_texture("sentinel_sword"), 4.0f, 0.85f, {0.f, 0.f}} {
	p_animations = {{"idle", {0, 6, 40, -1}},  {"run", {6, 4, 22, 5}},	  {"turn", {10, 3, 30, 0}},			{"jump", {13, 4, 22, -1}},		 {"land", {19, 3, 12, 0}},
					{"slash", {19, 3, 48, 0}}, {"swipe", {19, 3, 48, 0}}, {"charge_swipe", {17, 2, 80, 0}}, {"charge_slash", {22, 2, 80, 0}}};
	animation.set_params(get_params("idle"));
	get_collider().physics.air_friction = {0.95f, 0.999f};
	get_collider().physics.ground_friction = {0.95f, 0.999f};
	if (m_variant == SentinelVariant::knight) {
		m_shield.emplace(entity::FloatingPart(svc.assets.get_texture("sentinel_shield"), 2.0f, 0.65f, {0.f, 0.f}));
		m_shield->set_shield({24.f, 64.f}, {});
		m_shield->sprite->setOrigin({29.f, 64.f});
	}
	flags.state.set(StateFlags::vulnerable);
}

void Sentinel::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { m_services->soundboard.play_sound("tank_death", get_collider().get_center()); }
	Enemy::update(svc, map, player);
	if (died()) { return; }

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
	auto sword_offset = sf::Vector2f{-28.f * directions.actual.as_float(), 6.f} + tweak;
	auto shield_offset = sf::Vector2f{64.f * directions.actual.as_float(), 48.f};
	m_sword.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center() + sword_offset);
	if (m_shield) { m_shield->update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center() + shield_offset); }
	m_sword.sprite->setOrigin(m_sword.sprite->getLocalBounds().getCenter());
	m_sword.sprite->setTextureRect(sf::IntRect{{0, 0}, {84, 43}});
	m_sword.sprite->setRotation(sf::degrees(0));
	if (is_state(SentinelState::charge_slash)) { m_sword.sprite->setTextureRect(sf::IntRect{{0, 43}, {84, 43}}); }
	if (is_state(SentinelState::charge_swipe)) {
		m_sword.sprite->setTextureRect(sf::IntRect{{0, 43}, {84, 43}});
		if (directions.actual.left()) { m_sword.sprite->setRotation(sf::degrees(-90)); }
		if (directions.actual.right()) { m_sword.sprite->setRotation(sf::degrees(90)); }
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
			r = i == 1 ? 70.f : 60.f;
			attack.set_constant_radius(r);
			switch (i) {
			case 0: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{156.f * directions.actual.as_float(), 40.f}); break;
			case 1: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{106.f * directions.actual.as_float(), -20.f}); break;
			case 2: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{6.f * directions.actual.as_float(), -90.f}); break;
			}
			break;
		}
		if (animation.get_frame() == 19 && (is_state(SentinelState::swipe) || is_state(SentinelState::slash))) {
			attack.hit.activate();
		} else {
			attack.hit.deactivate();
		}
		attack.hurt_player(player, 2.f, {directions.actual.as_float() * 20.f, -0.4f});
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(SentinelState::turn); }

	state_function = state_function();
}

void Sentinel::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (is_state(SentinelState::slash) || is_state(SentinelState::swipe)) {
	} else {
		m_sword.render(svc, win, cam);
	}
	if (m_shield) { m_shield->render(svc, win, cam); }

	for (auto [i, attack] : std::views::enumerate(m_attacks)) {
		if (attack.hit.active()) { attack.render(win, cam); }
	}

	if (died()) { return; }
}

void Sentinel::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { debug(); }

fsm::StateFunction Sentinel::update_idle() {
	p_state.actual = SentinelState::idle;
	if (change_state(SentinelState::charge_slash, get_params("charge_slash"))) { return SENTINEL_BIND(update_charge_slash); }
	if (change_state(SentinelState::charge_swipe, get_params("charge_swipe"))) { return SENTINEL_BIND(update_charge_swipe); }
	if (change_state(SentinelState::turn, get_params("turn"))) { return SENTINEL_BIND(update_turn); }
	if (change_state(SentinelState::run, get_params("run"))) { return SENTINEL_BIND(update_run); }
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
	return SENTINEL_BIND(update_jump);
}

fsm::StateFunction Sentinel::update_land() {
	p_state.actual = SentinelState::land;
	return SENTINEL_BIND(update_land);
}

fsm::StateFunction Sentinel::update_swipe() {
	p_state.actual = SentinelState::swipe;
	if (animation.complete()) {
		request(SentinelState::idle);
		if (change_state(SentinelState::idle, get_params("idle"))) { return SENTINEL_BIND(update_idle); }
	}
	return SENTINEL_BIND(update_swipe);
}

fsm::StateFunction Sentinel::update_slash() {
	p_state.actual = SentinelState::slash;
	if (animation.complete()) {
		request(SentinelState::idle);
		if (change_state(SentinelState::idle, get_params("idle"))) { return SENTINEL_BIND(update_idle); }
	}
	return SENTINEL_BIND(update_slash);
}

fsm::StateFunction Sentinel::update_charge_swipe() {
	p_state.actual = SentinelState::charge_swipe;
	if (animation.complete()) {
		auto swipe_offset = sf::Vector2f{100.f * directions.actual.as_float(), -40.f};
		m_map->spawn_effect(*m_services, "sword_swipe", get_collider().get_center() + swipe_offset, {}, 0, -directions.actual.as_float());
		get_collider().physics.apply_force({80.f * directions.actual.as_float(), 0.f});
		request(SentinelState::swipe);
		if (change_state(SentinelState::swipe, get_params("swipe"))) { return SENTINEL_BIND(update_swipe); }
	}
	return SENTINEL_BIND(update_charge_swipe);
}

fsm::StateFunction Sentinel::update_charge_slash() {
	p_state.actual = SentinelState::charge_slash;
	if (animation.complete()) {
		auto swipe_offset = sf::Vector2f{100.f * directions.actual.as_float(), 0.f};
		m_map->spawn_effect(*m_services, "sword_slash", get_collider().get_center() + swipe_offset, {}, 0, -directions.actual.as_float());
		get_collider().physics.apply_force({80.f * directions.actual.as_float(), 0.f});
		request(SentinelState::slash);
		if (change_state(SentinelState::slash, get_params("slash"))) { return SENTINEL_BIND(update_slash); }
	}
	return SENTINEL_BIND(update_charge_slash);
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
		if (ImGui::Button("run")) { request(SentinelState::run); }
		if (ImGui::Button("charge_slash")) { request(SentinelState::charge_slash); }
		if (ImGui::Button("charge_swipe")) { request(SentinelState::charge_swipe); }
		ImGui::End();
	}
}

} // namespace fornani::enemy
