
#include <fornani/entities/enemy/boss/GrandMastiff.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto grand_mastiff_framerate = 12;

GrandMastiff::GrandMastiff(automa::ServiceProvider& svc, world::Map& map)
	: Boss{svc, map, "grand_mastiff"}, Animatable{svc, "enemy_grand_mastiff", {297, 256}}, m_demon_star(svc, "demon_star"), m_post_slash{400}, m_post_bite{1600}, m_post_howl{1800} {
	p_animations = {{"idle", {0, 4, grand_mastiff_framerate * 3, -1}},
					{"run", {4, 4, grand_mastiff_framerate * 2, 2}},
					{"growl", {15, 4, grand_mastiff_framerate * 7, 3}},
					{"turn", {10, 4, grand_mastiff_framerate * 2, 0}},
					{"howl", {0, 4, grand_mastiff_framerate * 12, 0}}};

	animation.set_params(get_params("idle"));
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
	}
	if (!has_flag_set(BossFlags::battle_mode)) { return; }

	if (secondary_collider) {
		get_secondary_collider().physics.position = get_collider().get_top() - get_secondary_collider().dimensions * 0.5f;
		get_secondary_collider().sync_components();
	}

	m_post_bite.update();
	m_post_howl.update();
	m_attack_timer.update();
	m_demon_star.update(svc, map, *this);

	face_player(player);
	flags.state.set(StateFlags::vulnerable);

	if (svc.ticker.every_second()) {
		if (random::percent_chance(30)) { request(GrandMastiffState::run); }
	}
	if (m_attack_timer.is_complete()) {
		m_attack_timer.start();
		// choose a random attack
		auto choice = random::random_range_float(0.f, 1.f);
		if (choice < 0.7f) {
			if (!m_post_bite.running()) { request(GrandMastiffState::growl); }
		} else {
			if (!m_post_howl.running()) { request(GrandMastiffState::howl); }
		}
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
	// debug();
}

void GrandMastiff::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (!has_flag_set(BossFlags::battle_mode)) { return; }
	Enemy::render(svc, win, cam);
	if (svc.greyblock_mode()) { m_bite.render(win, cam); }
}

fsm::StateFunction GrandMastiff::update_idle() {
	p_state.actual = GrandMastiffState::idle;
	if (change_state(GrandMastiffState::turn, get_params("turn"))) { return GRAND_MASTIFF_BIND(update_turn); }
	if (change_state(GrandMastiffState::run, get_params("run"))) { return GRAND_MASTIFF_BIND(update_run); }
	if (change_state(GrandMastiffState::wag, get_params("wag")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_wag); }
	if (change_state(GrandMastiffState::growl, get_params("growl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_growl); }
	if (change_state(GrandMastiffState::howl, get_params("howl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_howl); }
	return GRAND_MASTIFF_BIND(update_idle);
}

fsm::StateFunction GrandMastiff::update_run() {
	p_state.actual = GrandMastiffState::run;
	auto speed = animation.get_frame_count() == 3 || animation.get_frame_count() == 4 ? attributes.speed : attributes.speed * 0.5f;
	get_collider().physics.acceleration.x = directions.actual.as_float() * speed;
	if (change_state(GrandMastiffState::growl, get_params("growl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_growl); }
	if (change_state(GrandMastiffState::howl, get_params("howl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_howl); }
	if (animation.is_complete()) {
		if (change_state(GrandMastiffState::turn, get_params("turn"))) { return GRAND_MASTIFF_BIND(update_turn); }
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_run);
}

fsm::StateFunction GrandMastiff::update_bite() {
	p_state.actual = GrandMastiffState::bite;
	if (animation.just_started()) { p_services->soundboard.flags.mastiff.set(audio::Mastiff::growl); }
	get_collider().physics.acceleration.x = directions.actual.as_float() * 5.f;
	if (animation.get_frame_count() == 4) {
		m_bite.hit.activate();
		if (animation.keyframe_started()) { p_services->soundboard.flags.mastiff.set(audio::Mastiff::bite); }
	}
	if (animation.is_complete()) {
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_bite);
}

fsm::StateFunction GrandMastiff::update_turn() {
	p_state.actual = GrandMastiffState::turn;
	if (animation.complete()) {
		request_flip();
		if (change_state(GrandMastiffState::growl, get_params("growl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_growl); }
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_turn);
}

fsm::StateFunction GrandMastiff::update_slash() {
	p_state.actual = GrandMastiffState::slash;
	if (animation.is_complete()) {
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_slash);
}

fsm::StateFunction GrandMastiff::update_growl() {
	p_state.actual = GrandMastiffState::growl;
	if (animation.get_frame_count() == 1 && animation.keyframe_started()) { p_services->soundboard.play_sound("grand_mastiff_growl", get_collider().get_center()); }
	if (animation.get_frame_count() == 2 && animation.keyframe_started()) { spawn_bite(); }
	if (animation.is_complete()) {
		m_post_bite.start();
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	return GRAND_MASTIFF_BIND(update_growl);
}

fsm::StateFunction GrandMastiff::update_wag() {
	p_state.actual = GrandMastiffState::wag;
	if (animation.is_complete()) {
		request(GrandMastiffState::slash);
		if (change_state(GrandMastiffState::slash, get_params("slash"))) { return GRAND_MASTIFF_BIND(update_slash); }
	}
	return GRAND_MASTIFF_BIND(update_wag);
}

fsm::StateFunction GrandMastiff::update_howl() {
	p_state.actual = GrandMastiffState::howl;
	if (animation.just_started()) { m_howl_count.cancel(); }
	auto fire_rate = 70;
	if (m_howl_count.get_count() % fire_rate == 0) {
		auto xoffset = random::random_range_float(0.f, 680.f) * directions.actual.as_float();
		auto yoffset = random::random_range_float(-220.f, -190.f);
		auto offset = sf::Vector2f{xoffset, yoffset};
		auto randx = random::random_range_float(-1.f, 1.f);
		m_demon_star.get().set_barrel_point(get_collider().get_center() + offset);
		m_demon_star.shoot(*p_services, *p_map, sf::Vector2f{randx, 4.f});
	}
	if (animation.is_complete()) {
		m_post_howl.start();
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
	m_howl_count.update();
	return GRAND_MASTIFF_BIND(update_howl);
}

fsm::StateFunction GrandMastiff::update_die() {
	p_state.actual = GrandMastiffState::die;
	return GRAND_MASTIFF_BIND(update_die);
}

bool GrandMastiff::change_state(GrandMastiffState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
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
		ImGui::Text("Post Slash: %i", m_post_slash.get());
		ImGui::Text("Post Bite: %i", m_post_bite.get());
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("howl")) { request(GrandMastiffState::howl); }
		if (ImGui::Button("growl")) { request(GrandMastiffState::growl); }
		if (ImGui::Button("wag")) { request(GrandMastiffState::wag); }
		if (ImGui::Button("run")) { request(GrandMastiffState::run); }
		ImGui::End();
	}
}

} // namespace fornani::enemy
