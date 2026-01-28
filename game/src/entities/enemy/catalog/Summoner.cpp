
#include <fornani/entities/enemy/catalog/Summoner.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

bool b_summoner_debug{};
constexpr auto summoner_framerate = 16;

Summoner::Summoner(automa::ServiceProvider& svc, world::Map& map, int variant)
	: Enemy(svc, map, "summoner"), m_variant{static_cast<SummonerVariant>(variant)}, m_map{&map}, m_cooldowns{.post_summon{2400}, .walk{200}, .post_walk{1400}, .post_hurt{20}, .pulse{48}}, m_services{&svc}, m_attacks{.pulse{}},
	  m_pulse(svc, "pulse"), m_magic{svc, {40.f, 96.f}, colors::white, "guardian_magic"} {
	m_params = {{"idle", {0, 5, summoner_framerate * 2, -1}},
				{"walk", {5, 4, summoner_framerate * 2, -1}},
				{"begin_summon", {9, 7, summoner_framerate * 2, 0}},
				{"summon", {16, 5, summoner_framerate * 2, 3}},
				{"horizontal_pulse", {21, 4, summoner_framerate * 2, 0}},
				{"vertical_pulse", {25, 4, summoner_framerate * 2, 0}},
				{"turn", {29, 5, summoner_framerate * 2, 0}},
				{"dodge", {29, 5, summoner_framerate, 0}}};
	animation.set_params(get_params("idle"));
	flags.state.set(StateFlags::no_shake);
	flags.state.set(StateFlags::vulnerable);

	m_pulse.get().set_team(arms::Team::guardian);
	m_magic.deactivate();
	m_distant_range.set_dimensions({450, 800});
	get_collider().physics.set_friction_componentwise({0.95f, 0.99f});

	m_home = {std::numeric_limits<float>::max(), 0.f};
	for (auto& pt : map.home_points) {
		m_home.x = std::min(pt.x, m_home.x);
		m_home.y = std::max(pt.x, m_home.y);
	}
	flags.state.set(StateFlags::no_slowdown);
}

void Summoner::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { m_services->soundboard.play_sound("summoner_death", get_collider().get_center()); }

	m_cooldowns.post_summon.update();
	m_cooldowns.post_hurt.update();
	m_cooldowns.walk.update();
	m_cooldowns.post_walk.update();
	m_cooldowns.pulse.update();

	// positioning
	m_distant_range.set_position(get_collider().bounding_box.get_position() - (m_distant_range.get_dimensions() * 0.5f) + (get_collider().dimensions * 0.5f));

	Enemy::update(svc, map, player);
	face_player(player);

	// effects
	m_magic.set_position(get_collider().physics.position);
	m_magic.update(svc);

	m_pulse.update(svc, map, *this);
	m_player_position = player.get_collider().get_center();

	// melee
	for (auto& attack : m_attacks.pulse) {
		attack.set_constant_radius(16.f);
		auto damage = 1.f;
		attack.disable();
		attack.update();
		if (is_pulsing()) {
			attack.enable();
			if (animation.get_frame_count() > 1 || !m_cooldowns.pulse.running()) { attack.disable(); }
		}
		if (attack.hit.active()) {
			if (attack.hit.within_bounds(player.get_collider().bounding_box)) {
				if (!player.invincible()) { player.accumulated_forces.push_back({directions.actual.as_float() * 4.f, -2.f}); }
				player.hurt(damage);
			}
			for (auto& proj : map.active_projectiles) {
				if (proj.get_team() == arms::Team::guardian) { continue; }
				if (attack.hit.within_bounds(proj.get_collider())) {
					NANI_LOG_DEBUG(m_logger, ":FKAS:");
					map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_position()));
					random::percent_chance(50) ? svc.soundboard.flags.summoner.set(audio::Summoner::block_1) : svc.soundboard.flags.summoner.set(audio::Summoner::block_2);
					proj.destroy(false);
					svc.ticker.freeze_frame(3);
				}
			}
		}
	}

	auto can_summon = map.enemy_catalog.enemies.size() < 20;

	if (is_state(SummonerState::idle) && svc.ticker.every_second()) {
		if (!m_cooldowns.post_summon.running() && can_summon) {
			request(SummonerState::summon);
		} else {
			if (random::percent_chance(20) && !m_cooldowns.post_walk.running()) { request(SummonerState::walk); }
		}
	}
	if (player.get_collider().bounding_box.overlaps(get_collider().get_vicinity_rect()) && !m_cooldowns.post_walk.running()) { request(SummonerState::walk); }

	// caution
	auto incoming_projectile = m_caution.projectile_detected(map, physical.alert_range, arms::Team::skycorps);
	if (incoming_projectile.lnr != LNR::neutral) {
		if (incoming_projectile.lnr != directions.actual.lnr) {
			request(SummonerState::horizontal_pulse);
			if (is_hurt() && random::percent_chance(10) && can_summon) { request(SummonerState::summon); }
		}
	}

	if (incoming_projectile.und != UND::neutral) { request(SummonerState::vertical_pulse); }

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		if (!m_cooldowns.post_hurt.running()) { random::percent_chance(50) ? m_services->soundboard.flags.summoner.set(audio::Summoner::hurt_1) : m_services->soundboard.flags.summoner.set(audio::Summoner::hurt_2); }
		m_cooldowns.post_hurt.start();
		flags.state.reset(StateFlags::hurt);
	}

	// gameplay logic
	if (get_collider().get_center().x < m_home.x || get_collider().get_center().x > m_home.y) {
		request(SummonerState::walk);
		flags.state.set(StateFlags::out_of_zone);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(SummonerState::turn); }

	m_magic.deactivate();
	if (is_summoning() || is_pulsing()) { m_magic.activate(); }

	state_function = state_function();
}

void Summoner::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	for (auto& attack : m_attacks.pulse) {
		if (attack.hit.active()) {}
		// attack.render(win, cam);
	}

	m_magic.render(win, cam);
}

fsm::StateFunction Summoner::update_idle() {
	m_state.actual = SummonerState::idle;
	if (change_state(SummonerState::horizontal_pulse, get_params("horizontal_pulse"))) { return SUMMONER_BIND(update_horizontal_pulse); }
	if (change_state(SummonerState::vertical_pulse, get_params("vertical_pulse"))) { return SUMMONER_BIND(update_vertical_pulse); }
	if (change_state(SummonerState::turn, get_params("turn"))) { return SUMMONER_BIND(update_turn); }
	if (change_state(SummonerState::dodge, get_params("dodge"))) { return SUMMONER_BIND(update_dodge); }
	if (change_state(SummonerState::walk, get_params("walk"))) { return SUMMONER_BIND(update_walk); }
	if (change_state(SummonerState::summon, get_params("begin_summon"))) { return SUMMONER_BIND(update_begin_summon); }
	return SUMMONER_BIND(update_idle);
}

fsm::StateFunction Summoner::update_turn() {
	m_state.actual = SummonerState::turn;
	if (animation.complete()) {
		request_flip();
		if (change_state(SummonerState::horizontal_pulse, get_params("horizontal_pulse"))) { return SUMMONER_BIND(update_horizontal_pulse); }
		if (change_state(SummonerState::vertical_pulse, get_params("vertical_pulse"))) { return SUMMONER_BIND(update_vertical_pulse); }
		if (change_state(SummonerState::walk, get_params("walk"))) { return SUMMONER_BIND(update_walk); }
		if (change_state(SummonerState::summon, get_params("begin_summon"))) { return SUMMONER_BIND(update_begin_summon); }
		request(SummonerState::idle);
		if (change_state(SummonerState::idle, get_params("idle"))) { return SUMMONER_BIND(update_idle); }
	}
	return SUMMONER_BIND(update_turn);
}

fsm::StateFunction Summoner::update_dodge() {
	m_state.actual = SummonerState::dodge;
	get_collider().physics.acceleration.x = 12.f * directions.actual.as_float();
	if (animation.complete()) {
		request_flip();
		if (change_state(SummonerState::turn, get_params("turn"))) { return SUMMONER_BIND(update_turn); }
		if (change_state(SummonerState::horizontal_pulse, get_params("horizontal_pulse"))) { return SUMMONER_BIND(update_horizontal_pulse); }
		if (change_state(SummonerState::vertical_pulse, get_params("vertical_pulse"))) { return SUMMONER_BIND(update_vertical_pulse); }
		if (change_state(SummonerState::walk, get_params("walk"))) { return SUMMONER_BIND(update_walk); }
		if (change_state(SummonerState::summon, get_params("begin_summon"))) { return SUMMONER_BIND(update_begin_summon); }
		request(SummonerState::idle);
		if (change_state(SummonerState::idle, get_params("idle"))) { return SUMMONER_BIND(update_idle); }
	}
	return SUMMONER_BIND(update_dodge);
}

fsm::StateFunction Summoner::update_walk() {
	m_state.actual = SummonerState::walk;
	get_collider().physics.acceleration.x = directions.actual.as_float() * 1.f;
	if (animation.just_started()) { m_cooldowns.walk.start(); }
	if (change_state(SummonerState::horizontal_pulse, get_params("horizontal_pulse"))) {
		NANI_LOG_DEBUG(m_logger, ":dgsfgdsf:");
		return SUMMONER_BIND(update_horizontal_pulse);
	}
	if (change_state(SummonerState::vertical_pulse, get_params("vertical_pulse"))) { return SUMMONER_BIND(update_vertical_pulse); }
	if (m_cooldowns.walk.is_almost_complete()) {
		m_cooldowns.post_walk.start();
		if (change_state(SummonerState::dodge, get_params("dodge"))) { return SUMMONER_BIND(update_dodge); }
		if (change_state(SummonerState::turn, get_params("turn"))) { return SUMMONER_BIND(update_turn); }
		request(SummonerState::idle);
		if (change_state(SummonerState::idle, get_params("idle"))) { return SUMMONER_BIND(update_idle); }
	}
	return SUMMONER_BIND(update_walk);
}

fsm::StateFunction Summoner::update_horizontal_pulse() {
	m_state.actual = SummonerState::horizontal_pulse;
	if (animation.just_started()) {
		m_cooldowns.pulse.start();
		auto bp = get_collider().get_center();
		bp.x += 52.f * directions.actual.as_float();
		m_pulse.get().set_barrel_point(bp);
		m_map->spawn_projectile_at(*m_services, m_pulse.get(), m_pulse.get().get_barrel_point(), m_player_position - m_pulse.get().get_barrel_point());
		m_services->soundboard.flags.weapon.set(audio::Weapon::pulse);
	}
	for (auto [i, orb] : std::views::enumerate(m_attacks.pulse)) {
		auto offset = i != 1 ? 4.f : 0.f;
		orb.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * (48.f - offset), -8.f + (i - 1) * 32.f});
	}
	if (animation.is_complete()) {
		if (change_state(SummonerState::dodge, get_params("dodge"))) { return SUMMONER_BIND(update_dodge); }
		if (change_state(SummonerState::turn, get_params("turn"))) { return SUMMONER_BIND(update_turn); }
		if (change_state(SummonerState::vertical_pulse, get_params("vertical_pulse"))) { return SUMMONER_BIND(update_vertical_pulse); }
		request(SummonerState::idle);
		if (change_state(SummonerState::idle, get_params("idle"))) { return SUMMONER_BIND(update_idle); }
	}
	return SUMMONER_BIND(update_horizontal_pulse);
}

fsm::StateFunction Summoner::update_vertical_pulse() {
	m_state.actual = SummonerState::vertical_pulse;
	if (animation.just_started()) {
		m_cooldowns.pulse.start();
		auto bp = get_collider().get_center();
		bp.y -= 52.f;
		m_pulse.get().set_barrel_point(bp);
		m_map->spawn_projectile_at(*m_services, m_pulse.get(), m_pulse.get().get_barrel_point(), m_player_position - m_pulse.get().get_barrel_point());
		m_services->soundboard.flags.weapon.set(audio::Weapon::pulse);
	}
	for (auto [i, orb] : std::views::enumerate(m_attacks.pulse)) {
		auto offset = i != 1 ? 4.f : 0.f;
		orb.set_position(get_collider().get_center() + sf::Vector2f{(directions.actual.as_float() * 6.f) + (i - 1) * 32.f, -52.f + offset});
	}
	if (animation.is_complete()) {
		if (change_state(SummonerState::dodge, get_params("dodge"))) { return SUMMONER_BIND(update_dodge); }
		if (change_state(SummonerState::turn, get_params("turn"))) { return SUMMONER_BIND(update_turn); }
		if (change_state(SummonerState::horizontal_pulse, get_params("horizontal_pulse"))) { return SUMMONER_BIND(update_horizontal_pulse); }
		request(SummonerState::idle);
		if (change_state(SummonerState::idle, get_params("idle"))) { return SUMMONER_BIND(update_idle); }
	}
	return SUMMONER_BIND(update_vertical_pulse);
}

fsm::StateFunction Summoner::update_begin_summon() {
	m_state.actual = SummonerState::begin_summon;
	if (animation.is_complete()) {
		m_services->soundboard.flags.summoner.set(audio::Summoner::summon);
		request(SummonerState::summon);
		if (change_state(SummonerState::summon, get_params("summon"))) { return SUMMONER_BIND(update_summon); }
	}
	return SUMMONER_BIND(update_begin_summon);
}

fsm::StateFunction Summoner::update_summon() {
	m_state.actual = SummonerState::summon;
	if (animation.just_started()) { m_counters.summon.cancel(); }
	if (m_variant == SummonerVariant::mother) {
		auto fire_rate = 48;
		if (m_counters.summon.get_count() % fire_rate == 0) {
			auto xoffset = random::random_range_float(0.f, 280.f) * directions.actual.as_float();
			auto yoffset = random::random_range_float(-220.f, -190.f);
			auto offset = sf::Vector2f{xoffset, yoffset};
			m_pulse.get().set_barrel_point(get_collider().get_center() + offset);
			m_map->spawn_projectile_at(*m_services, m_pulse.get(), m_pulse.get().get_barrel_point(), m_player_position - m_pulse.get().get_barrel_point());
			m_services->soundboard.flags.weapon.set(audio::Weapon::pulse);
		}
	}
	if (m_variant == SummonerVariant::mage) {
		auto fire_rate = 72;
		if (m_counters.summon.get_count() % fire_rate == 0) {
			auto xoffset = random::random_range_float(0.f, 280.f) * directions.actual.as_float();
			auto yoffset = random::random_range_float(-220.f, -190.f);
			auto offset = sf::Vector2f{xoffset, yoffset};
			m_map->spawn_enemy(17, get_collider().get_center() + offset);
		}
	}
	m_counters.summon.update();
	if (animation.is_complete()) {
		m_cooldowns.post_summon.start();
		if (change_state(SummonerState::dodge, get_params("dodge"))) { return SUMMONER_BIND(update_dodge); }
		if (change_state(SummonerState::turn, get_params("turn"))) { return SUMMONER_BIND(update_turn); }
		if (change_state(SummonerState::horizontal_pulse, get_params("horizontal_pulse"))) { return SUMMONER_BIND(update_horizontal_pulse); }
		if (change_state(SummonerState::vertical_pulse, get_params("vertical_pulse"))) { return SUMMONER_BIND(update_vertical_pulse); }
		request(SummonerState::idle);
		if (change_state(SummonerState::idle, get_params("idle"))) { return SUMMONER_BIND(update_idle); }
	}
	return SUMMONER_BIND(update_summon);
}

bool Summoner::change_state(SummonerState next, anim::Parameters params) {
	if (m_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

void Summoner::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Lynx Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::Text("Post Summon: %i", m_cooldowns.post_summon.get());
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("summon")) { request(SummonerState::summon); }
		if (ImGui::Button("walk")) { request(SummonerState::walk); }
		if (ImGui::Button("horizontal pulse")) { request(SummonerState::horizontal_pulse); }
		if (ImGui::Button("vertical pulse")) { request(SummonerState::vertical_pulse); }
		if (ImGui::Button("summon")) { request(SummonerState::summon); }
		ImGui::End();
	}
}

} // namespace fornani::enemy
