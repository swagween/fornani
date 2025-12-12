
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Dash.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Dash::Dash(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction, bool omni)
	: Ability(svc, map, collider, direction), m_horizontal_multiplier{14.f}, m_vertical_multiplier{2.f}, m_rate{2}, m_omni{omni} {
	m_type = AbilityType::dash;
	m_state = AnimState::dash;
	if (omni) { m_state = direction.up() ? AnimState::dash_up : direction.down() ? AnimState::dash_down : AnimState::dash; }
	m_duration.start(64);
	map.effects.push_back(entity::Effect(svc, "small_flash", collider.get_center(), sf::Vector2f{collider.physics.apparent_velocity().x * 0.5f, 0.f}));
	svc.soundboard.flags.player.set(audio::Player::dash);
}

void Dash::update(shape::Collider& collider, PlayerController& controller) {
	if (m_direction.lnr != controller.last_requested_direction().as<LNR>() && collider.grounded()) {
		m_state = AnimState::turn_slide;
		m_horizontal_multiplier *= 0.99f;
		m_animation_trigger.start();
	}
	Ability::update(collider, controller);
	collider.flags.state.reset(shape::State::just_landed);
	auto const& vert = collider.acceleration_multiplier * m_vertical_multiplier;
	auto const& horiz = collider.acceleration_multiplier * m_horizontal_multiplier;
	auto terminal_vel = 100.f;
	if (collider.hit_ceiling_ramp()) {
		m_horizontal_multiplier *= 0.9f;
		m_vertical_multiplier *= 0.0f;
	}
	if (m_omni && m_direction.up_or_down()) {
		collider.physics.velocity.x = 0.f;
		collider.physics.acceleration.x = controller.horizontal_movement() * vert;
		collider.physics.velocity.x = controller.horizontal_movement() * vert;

		collider.physics.acceleration.y = m_direction.down() ? m_direction.as_float_und() * horiz * terminal_vel : m_direction.as_float_und() * horiz;
		collider.physics.velocity.y = m_direction.down() ? m_direction.as_float_und() * horiz * terminal_vel : m_direction.as_float_und() * horiz;
		if (m_duration.is_complete() && !m_direction.down()) { collider.physics.acceleration.y = 0.f; }
	} else {
		collider.physics.velocity.y = 0.f;
		collider.physics.acceleration.y = controller.vertical_movement() * vert;
		collider.physics.velocity.y = controller.vertical_movement() * vert;

		collider.physics.acceleration.x = m_direction.as_float() * horiz;
		collider.physics.velocity.x = m_direction.as_float() * horiz;
		if (m_duration.is_complete()) { collider.physics.acceleration.x = 0.f; }
	}
}

} // namespace fornani::player
