
#include "fornani/entities/enemy/catalog/Caster.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Caster::Caster(automa::ServiceProvider& svc, world::Map& map, int variant)
	: Enemy(svc, map, "caster"), m_services(&svc), m_map(&map), parts{.scepter{svc.assets.get_texture("caster_scepter"), 2.0f, 0.85f, {-16.f, 38.f}}, .wand{svc.assets.get_texture("caster_wand"), 2.0f, 0.85f, {-40.f, 48.f}}},
	  energy_ball(svc, "energy_ball"), m_variant{static_cast<CasterVariant>(variant)}, m_target_force{0.0003f}, m_debug{} {
	m_params = {{"idle", {0, 4, 28, -1}}, {"turn", {9, 3, 18, 0}}, {"prepare", {9, 3, 18, 0}}, {"signal", {4, 4, 28, 2}}, {"dormant", {8, 1, 32, -1}}};
	animation.set_params(get_params("dormant"));
	if (map.get_style_id() == 5) { cooldowns.awaken = util::Cooldown{4}; }
	get_collider().physics.set_friction_componentwise({0.964f, 0.964f});
	get_collider().flags.general.set(shape::General::complex);
	get_collider().set_flag(shape::ColliderFlags::simple);
	directions.desired.lnr = LNR::left;
	directions.actual.lnr = LNR::left;
	directions.movement.lnr = LNR::neutral;
	energy_ball.get().set_team(arms::Team::guardian);

	if (m_variant == CasterVariant::apprentice) { flags.general.reset(GeneralFlags::rare_drops); }

	cooldowns.awaken.start();
	parts.scepter.sprite->setTextureRect(sf::IntRect{{0, 0}, scepter_dimensions});
	parts.wand.sprite->setTextureRect(sf::IntRect{{0, 0}, wand_dimensions});
}

void Caster::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }
	get_collider().set_flag(shape::ColliderFlags::simple, !is_state(CasterState::dormant));
	if (!flags.state.test(StateFlags::invisible)) {
		// set target based on player position
		auto idle_distance = sf::Vector2f{-100.f * directions.actual.as_float(), -160.f};
		auto signal_distance = sf::Vector2f{300.f * directions.actual.as_float(), 0.f};
		auto standard_distance = sf::Vector2f{60.f * directions.actual.as_float(), -140.f};
		if (is_state(CasterState::idle)) {
			idle_distance.y = player.get_collider().grounded() ? -160.f : -120.f;
			if (random::percent_chance(12) && svc.ticker.every_x_ticks(10)) { idle_distance.x *= -1.f; }
			m_steering.seek(get_collider().physics, player.get_collider().get_center() + idle_distance, m_target_force);
		}
		if (is_state(CasterState::signal)) {
			signal_distance.y = player.get_collider().grounded() ? 0.f : -80.f;
			m_steering.seek(get_collider().physics, player.get_collider().get_center() + signal_distance, m_target_force);
		} else if (!is_dormant()) {
			m_steering.seek(get_collider().physics, player.get_collider().get_center() + standard_distance, m_target_force);
		}
	}

	energy_ball.update(svc, map, *this);
	auto bp = get_collider().get_center();
	bp.y -= 48.f;
	energy_ball.get().set_barrel_point(bp);
	attack_target = player.get_collider().get_center() - energy_ball.barrel_point();

	cooldowns.post_cast.update();
	if (flags.state.test(StateFlags::invisible)) {
		cooldowns.invisibility.update();
		if (cooldowns.invisibility.is_complete()) {
			flags.state.reset(StateFlags::invisible);
			m_map->effects.push_back(entity::Effect(*m_services, "small_flash", get_collider().physics.position));
		}
	}

	if (flags.state.test(StateFlags::invisible)) {
		flags.state.reset(StateFlags::vulnerable);
	} else {
		flags.state.set(StateFlags::vulnerable);
	}

	// reset animation states to determine next animation state
	face_player(player);
	directions.movement.lnr = get_collider().physics.velocity.x > 0.f ? LNR::right : LNR::left;
	if (!is_dormant()) {
		parts.scepter.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center());
		parts.wand.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center());
	}

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.play_sound("caster_hurt", get_collider().get_center());
		sound.hurt_sound_cooldown.start();
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
		cooldowns.pre_invisibility.start();
	}
	hurt_effect.update();
	if (cooldowns.pre_invisibility.is_almost_complete()) {
		teleport();
		cooldowns.pre_invisibility.cancel();
	}

	if (is_alert() && !is_hostile() && !cooldowns.post_cast.running()) { request(CasterState::prepare); }

	if (just_died()) { m_services->soundboard.flags.demon.set(audio::Demon::death); }

	if (directions.actual.lnr != directions.desired.lnr) { request(CasterState::turn); }

	state_function = state_function();
}

void Caster::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died() || is_state(CasterState::dormant) || flags.state.test(StateFlags::invisible)) { return; }
	m_variant == CasterVariant::apprentice ? parts.scepter.render(svc, win, cam) : parts.wand.render(svc, win, cam);
	if (svc.greyblock_mode()) {}
}

void Caster::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_debug) { debug(); }
}

void Caster::teleport() {
	auto original = get_collider().physics.position;
	bool done{};
	int ctr{};
	while (!done && ctr < 32) {
		auto attempt = random::random_vector_float(-300.f, 300.f);
		get_collider().physics.position += attempt;
		get_collider().sync_components();
		if (m_map->overlaps_middleground(get_collider().bounding_box)) {
			get_collider().physics.position = original;
			get_collider().sync_components();
		} else {
			done = true;
			m_map->effects.push_back(entity::Effect(*m_services, "medium_flash", original));
			m_services->soundboard.flags.world.set(audio::World::block_toggle);
			parts.wand.set_position(get_collider().get_center());
			parts.scepter.set_position(get_collider().get_center());
			flags.state.set(StateFlags::invisible);
			cooldowns.invisibility.start();
		}
		++ctr;
	}
}

fsm::StateFunction Caster::update_idle() {
	animation.label = "idle";
	p_state.actual = CasterState::idle;
	cooldowns.pre_invisibility.update();
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(CasterState::turn, get_params("turn"))) { return CASTER_BIND(update_turn); }
	if (change_state(CasterState::prepare, get_params("prepare"))) { return CASTER_BIND(update_prepare); }
	return CASTER_BIND(update_idle);
};

fsm::StateFunction Caster::update_turn() {
	animation.label = "turn";
	p_state.actual = CasterState::turn;
	cooldowns.pre_invisibility.update();
	if (animation.complete()) {
		request_flip();
		request(CasterState::idle);
		if (change_state(CasterState::idle, get_params("idle"))) { return CASTER_BIND(update_idle); }
	}
	return CASTER_BIND(update_turn);
}
fsm::StateFunction Caster::update_prepare() {
	p_state.actual = CasterState::prepare;
	if (animation.just_started()) { m_services->soundboard.play_sound("caster_scream", get_collider().get_center()); }
	cooldowns.pre_invisibility.update();
	if (animation.complete()) {
		request(CasterState::signal);
		if (change_state(CasterState::signal, get_params("signal"))) { return CASTER_BIND(update_signal); }
	}
	return CASTER_BIND(update_prepare);
};

fsm::StateFunction Caster::update_signal() {
	animation.label = "signal";
	p_state.actual = CasterState::signal;
	if (animation.just_started()) {
		auto sign = directions.actual.lnr == LNR::left ? 1.f : -1.f;
		parts.scepter.sprite->rotate(sf::degrees(90.f) * sign);
		cooldowns.rapid_fire.start(208);
	}
	if (m_services->ticker.every_x_ticks(20)) { flash.update(); }
	parts.scepter.sprite->setTextureRect(sf::IntRect{{0, scepter_dimensions.y + scepter_dimensions.y * flash.get_alternator()}, scepter_dimensions});
	parts.wand.sprite->setTextureRect(sf::IntRect{{0, wand_dimensions.y + wand_dimensions.y * flash.get_alternator()}, wand_dimensions});
	if (m_variant == CasterVariant::tyrant) { cooldowns.rapid_fire.update(); }
	if (cooldowns.rapid_fire.is_almost_complete()) {
		energy_ball.shoot(*m_services, *m_map, attack_target);
		cooldowns.rapid_fire.start();
	}
	if (animation.complete()) {
		parts.scepter.sprite->setTextureRect(sf::IntRect{{0, 0}, scepter_dimensions});
		parts.wand.sprite->setTextureRect(sf::IntRect{{0, 0}, wand_dimensions});
		auto sign = directions.actual.lnr == LNR::left ? 1.f : -1.f;
		parts.scepter.sprite->rotate(sf::degrees(-90.f) * sign);
		cooldowns.post_cast.start();
		if (m_variant == CasterVariant::apprentice) { energy_ball.shoot(*m_services, *m_map, attack_target); }
		if (change_state(CasterState::turn, get_params("turn"))) { return CASTER_BIND(update_turn); }
		request(CasterState::idle);
		if (change_state(CasterState::idle, get_params("idle"))) { return CASTER_BIND(update_idle); }
	}
	return CASTER_BIND(update_signal);
}

fsm::StateFunction Caster::update_dormant() {
	p_state.actual = CasterState::dormant;
	flags.state.reset(StateFlags::vulnerable);
	flags.general.reset(GeneralFlags::foreground);
	is_hostile() ? cooldowns.awaken.update() : cooldowns.awaken.reverse();
	if (cooldowns.awaken.halfway()) {
		shake();
		m_services->soundboard.flags.world.set(audio::World::pushable_move);
	}
	if (cooldowns.awaken.is_complete() || flags.state.test(StateFlags::shot)) {
		cooldowns.awaken.cancel();
		flags.state.set(StateFlags::vulnerable);
		m_map->effects.push_back(entity::Effect(*m_services, "small_explosion", get_collider().physics.position));
		m_services->soundboard.flags.world.set(audio::World::block_toggle);
		flags.general.set(GeneralFlags::foreground);
		request(CasterState::idle);
		if (change_state(CasterState::idle, get_params("idle"))) { return CASTER_BIND(update_idle); }
	}
	return CASTER_BIND(update_dormant);
}

bool Caster::change_state(CasterState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

void Caster::debug() {
	static auto sz = ImVec2{180.f, 450.f};
	static auto fric = 0.988f;
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Caster Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::Text("Animation: %s", animation.label.c_str());
		ImGui::SeparatorText("Controls");
		ImGui::SliderFloat("Friction", &fric, 0.9f, 0.999f);
		ImGui::SliderFloat("Target Force", &m_target_force, 0.0001f, 0.0005f, "%.4f");
		if (ImGui::Button("signal")) { request(CasterState::signal); }
		ImGui::End();
	}
	get_collider().physics.set_friction_componentwise({fric, fric});
}

} // namespace fornani::enemy
