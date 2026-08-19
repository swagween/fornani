
#include <fornani/entities/enemy/boss/GrandMastiff.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>
#include <cmath>

namespace fornani::enemy {

constexpr auto grand_mastiff_framerate = 12;

GrandMastiff::GrandMastiff(automa::ServiceProvider& svc, world::Map& map)
	: Boss{svc, map, "grand_mastiff"}, m_demon_star(svc, "demon_star"), m_post_slash{400}, m_post_bite{1600}, m_post_howl{1800}, m_moveset{GrandMastiffState::growl, GrandMastiffState::begin_howl, GrandMastiffState::slash},
	  m_move_counts{{GrandMastiffState::growl, 0}, {GrandMastiffState::begin_howl, 0}, {GrandMastiffState::slash, 0}, {GrandMastiffState::pound, 0}} {
	p_animatable.set_animations({{"idle", {0, 4, grand_mastiff_framerate * 3, -1}},
								 {"run", {4, 4, grand_mastiff_framerate * 2, 2}},
								 {"growl", {15, 4, grand_mastiff_framerate * 7, 3}},
								 {"turn", {10, 4, grand_mastiff_framerate * 2, 0}},
								 {"begin_howl", {25, 1, grand_mastiff_framerate * 12, 0}},
								 {"howl", {26, 4, grand_mastiff_framerate * 4, 3}},
								 {"pound", {30, 4, grand_mastiff_framerate * 5, 3}},
								 {"slash", {15, 7, grand_mastiff_framerate * 3, 0}}});

	p_animatable.animation.set_params(get_params("idle"));
	m_bite.hit.bounds.setRadius(48.f);
	m_demon_star.get().set_team(arms::Team::guardian);

	get_collider().physics.set_friction_componentwise({0.9f, 0.99f});
	flags.state.set(StateFlags::no_shake);
	if (secondary_collider) { get_secondary_collider().set_dimensions({160.f, 100.f}); }
}

void GrandMastiff::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Boss::update(svc, map, player);
	has_flag_set(BossFlags::battle_mode) ? flags.state.reset(StateFlags::intangible) : flags.state.set(StateFlags::intangible);
	if (consume_flag(BossFlags::start_battle)) {
		svc.data.switch_destructible_state(4013, true);
		svc.music_player.load(svc.finder, "tumult");
		svc.music_player.play_looped();
	}
	if (has_flag_set(BossFlags::end_battle) && !has_flag_set(BossFlags::post_death)) {
		svc.data.switch_destructible_state(4013, true);
		svc.music_player.pause();
		set_flag(BossFlags::post_death);
		svc.music_player.load(svc.finder, "none");
		svc.music_player.play_looped();
		svc.events.set_quest_progression_event.dispatch(13, 2);
		m_shockwaves.clear();
		map.clear_projectiles();
	}
	if (!has_flag_set(BossFlags::battle_mode)) { return; }

	if (secondary_collider) {
		get_secondary_collider().physics.position = get_collider().get_top() - get_secondary_collider().dimensions * 0.5f;
		get_secondary_collider().sync_components();
	}

	m_post_bite.update();
	m_post_howl.update();
	m_demon_star.update(svc, map, *this);

	// shockwaves
	if (Boss::has_flag_set(BossFlags::battle_mode)) {
		for (auto& s : m_shockwaves) {
			if (s.hit.active()) { player.hurt(); }
			s.update(svc, map);
			s.handle_player(player);
		}
	}
	std::erase_if(m_shockwaves, [](auto const& s) { return s.lifetime.is_almost_complete(); });

	// attacks
	for (auto [i, attack] : std::views::enumerate(m_attacks)) {
		auto r = i == 1 ? 100.f : 80.f;
		attack.set_constant_radius(r);
		switch (p_state.actual) {
		case GrandMastiffState::slash:
			r = i == 1 ? 60.f : 40.f;
			attack.set_constant_radius(r);
			switch (i) {
			case 0: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{156.f * directions.actual.as_float(), 20.f}); break;
			case 1: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{176.f * directions.actual.as_float(), -60.f}); break;
			case 2: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{168.f * directions.actual.as_float(), -105.f}); break;
			}
			break;
		case GrandMastiffState::pound:
			r = i == 1 ? 50.f : 40.f;
			attack.set_constant_radius(r);
			switch (i) {
			case 0: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{0.f * directions.actual.as_float(), 54.f}); break;
			case 1: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{-26.f * directions.actual.as_float(), 54.f}); break;
			case 2: attack.set_position(Enemy::get_collider().get_center() + sf::Vector2f{26.f * directions.actual.as_float(), 54.f}); break;
			}
			break;
		}
		if ((p_animatable.animation.get_frame() == 20 && is_state(GrandMastiffState::slash)) || (p_animatable.animation.get_frame() == 32 && is_state(GrandMastiffState::pound))) {
			attack.hit.activate();
		} else {
			attack.hit.deactivate();
		}
		if (attack.hurt_player(player, 2.f, {directions.actual.as_float() * 20.f, -0.4f})) { player.controller.flush_ability(); }
	}

	face_player(player);
	flags.state.set(StateFlags::vulnerable);

	// moves
	if (svc.ticker.every_second()) {
		if (random::percent_chance(30)) { request(GrandMastiffState::run); }
	}
	if (half_health()) {
		if (!std::ranges::contains(m_moveset, GrandMastiffState::pound)) { m_moveset.push_back(GrandMastiffState::pound); }
	}
	if (m_attack_timer.is_complete()) {
		std::vector<GrandMastiffState> available_moves{};
		std::ranges::copy_if(m_moveset, std::back_inserter(available_moves), [this](auto const move) { return move != m_last_move; });

		std::vector<double> weights{};
		for (auto const move : available_moves) { weights.push_back(1.0 / std::pow(1.0 + m_move_counts[move], 2.0)); }
		std::discrete_distribution<std::size_t> dist(weights.begin(), weights.end());
		auto const move = available_moves[dist(random::engine())];

		request(move);
		m_last_move = move;
		m_attack_timer.start();
		++m_move_counts[move];
	}

	m_bite.hit.deactivate();
	m_bite.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 32.f, -18.f});
	if (m_bite_effect) {
		m_bite_effect->tick();
		if (m_bite_effect->is_complete()) { m_bite_effect.reset(); }
		if (m_bite_effect->animation.get_frame_count() == 4) { svc.soundboard.play_sound("grand_mastiff_bite"); }
		if (m_bite_effect->animation.get_frame_count() == 8) { m_bite.hit.activate(); }
		if (m_bite_effect->animation.get_frame_count() < 6) { m_bite_target = player.get_center(); }
	}
	m_bite.set_position(m_bite_target);
	m_player_position = player.get_center();

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(GrandMastiffState::turn); }

	state_function = state_function();

	if (m_bite.hit.within_bounds(player.hurtbox) && m_bite.hit.active() && !health.is_dead()) { player.hurt(); }
}

void GrandMastiff::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	if (m_bite_effect) {
		m_bite_effect->set_position(m_bite.hit.bounds.getPosition() - cam);
		win.draw(*m_bite_effect);
	}
	if (debug::is_debug()) { debug(); }
}

void GrandMastiff::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (!has_flag_set(BossFlags::battle_mode)) { return; }
	Enemy::render(svc, win, cam);
	if (!debug::is_production()) {
		m_bite.render(win, cam);
		for (auto [i, attack] : std::views::enumerate(m_attacks)) {
			if (attack.hit.active()) { attack.render(win, cam); }
		}
	}
}

fsm::StateFunction GrandMastiff::update_idle() {
	p_state.actual = GrandMastiffState::idle;
	m_attack_timer.update();
	if (change_state(GrandMastiffState::turn, get_params("turn"))) { return GRAND_MASTIFF_BIND(update_turn); }
	if (change_state(GrandMastiffState::run, get_params("run"))) { return GRAND_MASTIFF_BIND(update_run); }
	if (change_state(GrandMastiffState::wag, get_params("wag")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_wag); }
	if (change_state(GrandMastiffState::growl, get_params("growl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_growl); }
	if (change_state(GrandMastiffState::begin_howl, get_params("begin_howl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_begin_howl); }
	if (change_state(GrandMastiffState::pound, get_params("pound")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_pound); }
	if (change_state(GrandMastiffState::slash, get_params("slash")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_slash); }
	return GRAND_MASTIFF_BIND(update_idle);
}

fsm::StateFunction GrandMastiff::update_run() {
	p_state.actual = GrandMastiffState::run;
	m_attack_timer.update();
	auto speed = p_animatable.animation.get_frame_count() == 3 || p_animatable.animation.get_frame_count() == 4 ? attributes.speed : attributes.speed * 0.5f;
	get_collider().physics.acceleration.x = directions.actual.as_float() * speed;
	if (p_animatable.animation.get_frame_count() == 2 && p_animatable.animation.keyframe_started()) {
		p_services->soundboard.play_sound("thud", get_collider().get_center());
		p_services->camera_controller.shake(10, 0.2f, 200, 20);
	}
	if (change_state(GrandMastiffState::growl, get_params("growl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_growl); }
	if (change_state(GrandMastiffState::begin_howl, get_params("begin_howl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_begin_howl); }
	if (change_state(GrandMastiffState::pound, get_params("pound")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_pound); }
	if (change_state(GrandMastiffState::slash, get_params("slash")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_slash); }
	if (p_animatable.animation.is_complete()) {
		if (change_state(GrandMastiffState::turn, get_params("turn"))) { return GRAND_MASTIFF_BIND(update_turn); }
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_run);
}

fsm::StateFunction GrandMastiff::update_bite() {
	p_state.actual = GrandMastiffState::bite;
	if (p_animatable.animation.just_started()) { p_services->soundboard.flags.mastiff.set(audio::Mastiff::growl); }
	get_collider().physics.acceleration.x = directions.actual.as_float() * 5.f;
	if (p_animatable.animation.get_frame_count() == 4) {
		m_bite.hit.activate();
		if (p_animatable.animation.keyframe_started()) { p_services->soundboard.flags.mastiff.set(audio::Mastiff::bite); }
	}
	if (p_animatable.animation.is_complete()) {
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_bite);
}

fsm::StateFunction GrandMastiff::update_turn() {
	p_state.actual = GrandMastiffState::turn;
	m_attack_timer.update();
	if (p_animatable.animation.complete()) {
		request_flip();
		if (change_state(GrandMastiffState::growl, get_params("growl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_growl); }
		if (change_state(GrandMastiffState::pound, get_params("pound")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_pound); }
		if (change_state(GrandMastiffState::slash, get_params("slash")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_slash); }
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_turn);
}

fsm::StateFunction GrandMastiff::update_slash() {
	p_state.actual = GrandMastiffState::slash;
	if (p_animatable.frame_action(5)) { p_services->soundboard.play_sound("claw_slash", get_collider().get_center()); }
	if (p_animatable.animation.is_complete()) {
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_slash);
}

fsm::StateFunction GrandMastiff::update_growl() {
	p_state.actual = GrandMastiffState::growl;
	if (p_animatable.animation.get_frame_count() == 1 && p_animatable.animation.keyframe_started()) { p_services->soundboard.play_sound("grand_mastiff_growl", get_collider().get_center()); }
	if (p_animatable.animation.get_frame_count() == 2 && p_animatable.animation.keyframe_started()) { spawn_bite(); }
	if (p_animatable.animation.is_complete()) {
		m_post_bite.start();
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_growl);
}

fsm::StateFunction GrandMastiff::update_wag() {
	p_state.actual = GrandMastiffState::wag;
	if (p_animatable.animation.is_complete()) {
		request(GrandMastiffState::slash);
		if (change_state(GrandMastiffState::slash, get_params("slash"))) { return GRAND_MASTIFF_BIND(update_slash); }
	}
	return GRAND_MASTIFF_BIND(update_wag);
}

fsm::StateFunction GrandMastiff::update_begin_howl() {
	p_state.actual = GrandMastiffState::begin_howl;
	if (p_animatable.animation.is_complete()) {
		request(GrandMastiffState::howl);
		if (change_state(GrandMastiffState::howl, get_params("howl"))) { return GRAND_MASTIFF_BIND(update_howl); }
	}
	return GRAND_MASTIFF_BIND(update_begin_howl);
}

fsm::StateFunction GrandMastiff::update_howl() {
	p_state.actual = GrandMastiffState::howl;
	if (p_animatable.animation.just_started()) {
		m_howl_count.cancel();
		p_services->soundboard.play_sound("grand_mastiff_howl", get_collider().get_center());
	}
	auto fire_rate = 30;
	if (m_howl_count.get_count() % fire_rate == 0) {
		auto xoffset = random::random_range_float(-480.f, 680.f) * directions.actual.as_float();
		auto yoffset = random::random_range_float(-260.f, -220.f);
		auto offset = sf::Vector2f{xoffset, yoffset};
		auto randx = random::random_range_float(-1.f, 1.f);
		m_demon_star.get().set_barrel_point(get_collider().get_center() + offset);
		m_demon_star.shoot(*p_services, *p_map, sf::Vector2f{randx, 4.f});
	}
	if (p_animatable.animation.is_complete()) {
		m_post_howl.start();
		m_attack_timer.start();
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	m_howl_count.update();
	return GRAND_MASTIFF_BIND(update_howl);
}

fsm::StateFunction GrandMastiff::update_pound() {
	p_state.actual = GrandMastiffState::pound;
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("grand_mastiff_bark", get_collider().get_center()); }
	if (p_animatable.animation.get_frame_count() == 1 && p_animatable.animation.keyframe_started()) {
		p_services->soundboard.play_sound("heavy_land", get_collider().get_center());
		p_services->camera_controller.shake(10, 0.4f, 200, 20);
		for (int i = 0; i < 2; ++i) {
			auto spd = i == 0 ? 1.f : 1.5f;
			auto lifetime = 1 == 0 ? 800 : 500;
			m_shockwaves.push_back(entity::Shockwave{{20, lifetime, 2, {spd * directions.actual.as_float(), 0.f}}});
			m_shockwaves.back().origin = Enemy::get_collider().physics.position + Enemy::get_collider().bounding_box.get_dimensions();
			m_shockwaves.back().start();
		}
	}
	if (p_animatable.animation.is_complete()) {
		request(GrandMastiffState::slash);
		if (change_state(GrandMastiffState::slash, get_params("slash"))) { return GRAND_MASTIFF_BIND(update_slash); }
	}
	return GRAND_MASTIFF_BIND(update_pound);
}

fsm::StateFunction GrandMastiff::update_die() {
	p_state.actual = GrandMastiffState::die;
	return GRAND_MASTIFF_BIND(update_die);
}

bool GrandMastiff::change_state(GrandMastiffState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

void GrandMastiff::spawn_bite() {
	m_bite_effect.emplace(*p_services, "mastiff_bite", sf::Vector2i{167, 167});
	m_bite_effect->center();
	m_bite_effect->push_and_set_animation("bite", {0, 12, 24, 0});
}

void GrandMastiff::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Grand Mastiff Debug")) {
		if (ImGui::Button("Start Battle")) { start_battle(); }
		ImGui::SeparatorText("Info");
		ImGui::Text("Moveset: %i", m_moveset.size());
		ImGui::Text("Last Move: %i", m_last_move);
		ImGui::Text("Post Slash: %i", m_post_slash.get());
		ImGui::Text("Post Bite: %i", m_post_bite.get());
		ImGui::Text("Attack Timer: ");
		ImGui::ProgressBar(m_attack_timer.get_normalized());
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("howl")) { request(GrandMastiffState::begin_howl); }
		if (ImGui::Button("growl")) { request(GrandMastiffState::growl); }
		if (ImGui::Button("wag")) { request(GrandMastiffState::wag); }
		if (ImGui::Button("run")) { request(GrandMastiffState::run); }
		if (ImGui::Button("pound")) { request(GrandMastiffState::pound); }
		ImGui::SeparatorText("Move Counts");
		ImGui::Text("Slash: %i", m_move_counts[GrandMastiffState::slash]);
		ImGui::Text("Howl: %i", m_move_counts[GrandMastiffState::begin_howl]);
		ImGui::Text("Pound: %i", m_move_counts[GrandMastiffState::pound]);
		ImGui::Text("Growl: %i", m_move_counts[GrandMastiffState::growl]);
		ImGui::End();
	}
}

} // namespace fornani::enemy
