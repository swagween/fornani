
#include <fornani/entities/enemy/boss/GrandMastiff.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto grand_mastiff_framerate = 14;

GrandMastiff::GrandMastiff(automa::ServiceProvider& svc, world::Map& map) : Boss{svc, map, "grand_mastiff"}, m_post_slash{400}, m_post_bite{600} {
	m_params = {{"idle", {0, 4, grand_mastiff_framerate * 3, -1}}, {"run", {4, 6, grand_mastiff_framerate, 2}}, {"growl", {15, 4, grand_mastiff_framerate * 2, 0}}, {"turn", {10, 5, grand_mastiff_framerate * 2, 0}}};

	animation.set_params(get_params("idle"));
	m_bite.hit.bounds.setRadius(40.f);

	get_collider().physics.set_friction_componentwise({0.9f, 0.99f});
}

void GrandMastiff::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);

	if (is_hostile()) { request(GrandMastiffState::bite); }

	m_bite.hit.deactivate();
	m_bite.hit.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 32.f, -18.f});

	if (svc.ticker.every_second()) {
		if (random::percent_chance(30)) { request(GrandMastiffState::run); }
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(GrandMastiffState::turn); }

	state_function = state_function();

	if (m_bite.hit.within_bounds(player.get_collider().hurtbox) && m_bite.hit.active() && !health.is_dead()) { player.hurt(); }
}

void GrandMastiff::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	debug();
}

void GrandMastiff::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
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
	if (animation.is_complete()) {
		if (change_state(GrandMastiffState::growl, get_params("growl")) && get_collider().grounded()) { return GRAND_MASTIFF_BIND(update_growl); }
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
	if (animation.is_complete()) {
		request(GrandMastiffState::bite);
		if (change_state(GrandMastiffState::bite, get_params("bite"))) { return GRAND_MASTIFF_BIND(update_bite); }
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
	if (animation.is_complete()) {
		request(GrandMastiffState::idle);
		if (change_state(GrandMastiffState::idle, get_params("idle"))) { return GRAND_MASTIFF_BIND(update_idle); }
	}
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

void GrandMastiff::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Grand Mastiff Debug")) {
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
