
#include <fornani/entities/enemy/catalog/Spitefly.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto spitefly_framerate = 12;

Spitefly::Spitefly(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, "spitefly"), m_services{&svc} {
	m_params = {{"idle", {0, 4, spitefly_framerate * 2, -1}}, {"turn", {4, 1, spitefly_framerate * 2, 0}}, {"sleep", {5, 1, spitefly_framerate * 2, -1}}, {"awaken", {6, 7, spitefly_framerate * 2, 0}}};
	animation.set_params(get_params("sleep"));
	p_state.actual = SpiteflyState::sleep;
	flags.general.set(GeneralFlags::hurt_on_contact);

	flags.general.reset(GeneralFlags::gravity);
	flags.state.set(StateFlags::simple_physics);

	m_variant = static_cast<SpiteflyVariant>(variant);
	if (is_bomb()) {
		m_bomb = entity::WeaponPackage{svc, "demon_bomb"};
		m_bomb_part = entity::FloatingPart{svc.assets.get_texture("demon_bomb"), 0.9f, 0.9f, {0.f, 32.f}};
		m_bomb->get().set_team(arms::Team::guardian);
	}

	is_albino() ? collider.physics.set_friction_componentwise({0.99f, 0.99f}) : collider.physics.set_friction_componentwise({0.99f, 0.99f});
	if (is_albino()) {
		flags.general.set(GeneralFlags::custom_channels);
		m_custom_channel = EnemyChannel::invincible;
		flags.general.reset(GeneralFlags::map_collision);
		animation.set_params(get_params("awaken"));
		p_state.actual = SpiteflyState::awaken;
		state_function = std::bind(&Spitefly::update_awaken, this);
		attributes.base_hp = 8;
		health.set_max(8);
	}
}

void Spitefly::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	Enemy::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);

	// bomb variant stuff
	if (m_bomb) { m_bomb->update(svc, map, *this); }
	if (m_bomb_part) { m_bomb_part->update(svc, map, player, directions.actual, Drawable::get_scale(), collider.get_center()); }

	if (is_active()) {
		auto force = is_albino() ? 0.00012f : 0.0001f;
		m_steering.seek(Enemy::collider.physics, player.collider.get_center(), force);
		if (is_bomb()) {
			if (m_bomb) {
				if (svc.ticker.every_x_ticks(1000)) {
					auto bp = sf::Vector2f{0.f, 32.f};
					m_bomb->get().set_barrel_point(collider.get_center() + bp);
					map.spawn_projectile_at(svc, m_bomb->get(), collider.get_center() + bp, player.collider.get_center() - collider.get_center() + bp);
				}
			}
		}
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(SpiteflyState::turn); }
	if ((is_hostile() && is_state(SpiteflyState::sleep)) || is_hurt()) { request(SpiteflyState::awaken); }

	state_function = state_function();
}

void Spitefly::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
	// bomb variant stuff
	if (is_active()) {
		if (m_bomb_part) { m_bomb_part->render(svc, win, cam); }
	}
}

fsm::StateFunction Spitefly::update_idle() {
	p_state.actual = SpiteflyState::idle;
	if (animation.get_frame() == 0 && animation.keyframe_started()) { collider.physics.acceleration.y -= is_bomb() ? 1.6f : 0.8f; }
	if (change_state(SpiteflyState::turn, get_params("turn"))) { return SPITEFLY_BIND(update_turn); }
	return SPITEFLY_BIND(update_idle);
}

fsm::StateFunction Spitefly::update_sleep() {
	p_state.actual = SpiteflyState::sleep;
	if (change_state(SpiteflyState::awaken, get_params("awaken"))) { return SPITEFLY_BIND(update_awaken); }
	return SPITEFLY_BIND(update_sleep);
}

fsm::StateFunction Spitefly::update_awaken() {
	p_state.actual = SpiteflyState::awaken;
	if (animation.is_complete()) {
		flags.general.set(GeneralFlags::gravity);
		request(SpiteflyState::idle);
		if (change_state(SpiteflyState::idle, get_params("idle"))) { return SPITEFLY_BIND(update_idle); }
	}
	return SPITEFLY_BIND(update_awaken);
}

fsm::StateFunction Spitefly::update_turn() {
	p_state.actual = SpiteflyState::turn;
	if (animation.complete()) {
		request_flip();
		request(SpiteflyState::idle);
		if (change_state(SpiteflyState::idle, get_params("idle"))) { return SPITEFLY_BIND(update_idle); }
	}
	return SPITEFLY_BIND(update_turn);
}

bool Spitefly::change_state(SpiteflyState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
