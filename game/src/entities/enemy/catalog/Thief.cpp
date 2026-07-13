
#include <fornani/entities/enemy/catalog/Thief.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Thief::Thief(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "thief"), m_services(&svc), m_map(&map), m_respawn{400} {
	auto fr = 22;
	p_animatable.set_animations({{"dive", {0, 1, fr, -1}},
								 {"land", {1, 3, fr, 0}},
								 {"prepare", {4, 1, fr * 2, 0}},
								 {"dash", {5, 1, fr * 3, 0}},
								 {"stop", {6, 2, fr, 0}},
								 {"laugh", {8, 2, fr, 3}},
								 {"escape", {10, 2, fr * 2, 0, true}},
								 {"hurt", {12, 1, fr * 4, 0}}});
	p_animatable.animation.set_params(get_params("dive"));
	get_collider().physics.set_friction_componentwise({0.9f, 0.9f});
	attributes.team = arms::Team::beast;
	get_collider().set_flag(shape::ColliderFlags::simple);
	flags.state.set(StateFlags::invisible);
	flags.state.set(StateFlags::intangible);
	flags.state.set(StateFlags::vulnerable);
	m_respawn.start();
	m_flags.set(ThiefFlags::hiding);
}

void Thief::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }

	m_respawn.update();

	// logic
	if (m_respawn.running() || m_flags.test(ThiefFlags::hiding)) { set_direction(player_behind(player) ? LR::left : LR::right); }
	if (get_collider().bounding_box.overlaps(player.hurtbox) && (is_state(ThiefState::dash) || is_state(ThiefState::stop)) && !m_flags.test(ThiefFlags::succeeded)) {
		if (player.has_item("gas_mask")) {
			svc.events.remove_item_event.dispatch(svc, "gas_mask");
			m_services->ticker.freeze_frame(24, 0.02f);
			m_loot.emplace(svc, "gas_mask");
			m_loot->center();
			m_flags.set(ThiefFlags::succeeded);
			svc.soundboard.play_sound("arms_catch_tomahawk");
		}
	}
	if (m_flags.test(ThiefFlags::succeeded)) { attributes.treasure_chance = 1.0f; }

	hurt_effect.update();
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) { random::coin_flip() ? svc.soundboard.play_sound("thief_haah", get_collider().get_center()) : svc.soundboard.play_sound("thief_harumph", get_collider().get_center()); }
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
		request(ThiefState::hurt);
	}

	if (is_hostile() && is_state(ThiefState::dive) && !m_respawn.running()) {
		if (!m_flags.test(ThiefFlags::dive_in)) {
			map.spawn_effect(*m_services, "medium_flash", get_collider().get_center());
			svc.soundboard.play_sound("thief_hurah", get_collider().get_center());
			m_flags.reset(ThiefFlags::hiding);
		}
		m_flags.set(ThiefFlags::dive_in);
		flags.state.reset(StateFlags::invisible);
		flags.state.reset(StateFlags::intangible);
	} else {
		m_flags.reset(ThiefFlags::dive_in);
	}

	state_function = state_function();
}

void Thief::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died() || flags.state.test(StateFlags::invisible)) { return; }
	if (m_loot) {
		m_loot->set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 8.f, -8.f} - cam);
		win.draw(*m_loot);
	}
}

fsm::StateFunction Thief::update_dive() {
	p_state.actual = ThiefState::dive;
	if (m_flags.test(ThiefFlags::dive_in)) { get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * 0.2f, 1.f} * attributes.speed; }
	if (get_collider().grounded()) {
		request(ThiefState::land);
		if (change_state(ThiefState::land, get_params("land"))) { return THIEF_BIND(update_land); }
	}
	return THIEF_BIND(update_dive);
}

fsm::StateFunction Thief::update_land() {
	p_state.actual = ThiefState::land;
	if (p_animatable.animation.complete()) {
		if (m_flags.test(ThiefFlags::succeeded)) {
			request(ThiefState::laugh);
			if (change_state(ThiefState::laugh, get_params("laugh"))) { return THIEF_BIND(update_laugh); }
		}
		request(ThiefState::prepare);
		if (change_state(ThiefState::prepare, get_params("prepare"))) { return THIEF_BIND(update_prepare); }
	}
	return THIEF_BIND(update_land);
}

fsm::StateFunction Thief::update_prepare() {
	p_state.actual = ThiefState::prepare;
	if (p_animatable.animation.complete()) {
		request(ThiefState::dash);
		if (change_state(ThiefState::dash, get_params("dash"))) { return THIEF_BIND(update_dash); }
	}
	return THIEF_BIND(update_prepare);
}

fsm::StateFunction Thief::update_dash() {
	p_state.actual = ThiefState::dash;
	if (p_animatable.animation.just_started()) { m_services->soundboard.play_sound("thief_swipe", get_collider().get_center()); }
	get_collider().physics.velocity.x = directions.actual.as_float() * attributes.speed * 2.f;
	if (p_animatable.animation.complete() || m_flags.test(ThiefFlags::succeeded)) {
		request(ThiefState::stop);
		if (change_state(ThiefState::stop, get_params("stop"))) { return THIEF_BIND(update_stop); }
	}
	return THIEF_BIND(update_dash);
}

fsm::StateFunction Thief::update_stop() {
	p_state.actual = ThiefState::stop;
	if (p_animatable.animation.complete()) {
		if (!m_flags.test(ThiefFlags::succeeded)) { random::coin_flip() ? m_services->soundboard.play_sound("thief_whiff_1", get_collider().get_center()) : m_services->soundboard.play_sound("thief_whiff", get_collider().get_center()); }
		m_flags.consume(ThiefFlags::succeeded) ? request(ThiefState::laugh) : request(ThiefState::escape);
		if (change_state(ThiefState::escape, get_params("escape"))) { return THIEF_BIND(update_escape); }
		if (change_state(ThiefState::laugh, get_params("laugh"))) { return THIEF_BIND(update_laugh); }
	}
	return THIEF_BIND(update_stop);
}

fsm::StateFunction Thief::update_laugh() {
	p_state.actual = ThiefState::laugh;
	if (p_animatable.animation.just_started()) { m_services->soundboard.play_sound("thief_laugh", get_collider().get_center()); }
	if (change_state(ThiefState::hurt, get_params("hurt"))) { return THIEF_BIND(update_hurt); }
	if (p_animatable.animation.complete()) {
		request(ThiefState::escape);
		if (change_state(ThiefState::escape, get_params("escape"))) { return THIEF_BIND(update_escape); }
	}
	return THIEF_BIND(update_laugh);
}

fsm::StateFunction Thief::update_escape() {
	p_state.actual = ThiefState::escape;
	if (p_animatable.animation.get_frame_count() == 1) { get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float(), -1.f} * attributes.speed; }
	if (p_animatable.animation.is_complete()) {
		teleport();
		request(ThiefState::dive);
		if (change_state(ThiefState::dive, get_params("dive"))) { return THIEF_BIND(update_dive); }
	}
	return THIEF_BIND(update_escape);
}

fsm::StateFunction Thief::update_hurt() {
	p_state.actual = ThiefState::hurt;
	if (p_animatable.animation.complete()) {
		request(ThiefState::escape);
		if (change_state(ThiefState::escape, get_params("escape"))) { return THIEF_BIND(update_escape); }
	}
	return THIEF_BIND(update_hurt);
}

void Thief::teleport() {
	m_map->spawn_effect(*m_services, "medium_flash", get_collider().get_center());
	m_services->soundboard.flags.world.set(audio::World::block_toggle);
	flags.state.set(StateFlags::invisible);
	flags.state.set(StateFlags::intangible);
	Enemy::set_position(m_map->get_closest_home_point(get_collider().get_center()));
	m_respawn.start();
}

bool Thief::change_state(ThiefState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
