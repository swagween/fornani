
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Dash.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Dash::Dash(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction) : Ability(svc, map, collider, direction), m_horizontal_multiplier{14.f}, m_vertical_multiplier{2.f}, m_rate{2} {
	m_type = AbilityType::dash;
	m_state = AnimState::dash;
	m_duration.start(64);
	map.effects.push_back(entity::Effect(svc, "small_flash", collider.get_center(), sf::Vector2f{collider.physics.velocity.x * 0.5f, 0.f}, 0, 6));
	svc.soundboard.flags.player.set(audio::Player::dash);
}

void Dash::update(shape::Collider& collider, PlayerController& controller) {
	Ability::update(collider, controller);
	collider.physics.velocity.y = 0.f;
	collider.physics.acceleration.y = controller.vertical_movement() * m_vertical_multiplier;
	collider.physics.velocity.y = controller.vertical_movement() * m_vertical_multiplier;

	collider.physics.acceleration.x = m_direction.as_float() * m_horizontal_multiplier;
	collider.physics.velocity.x = m_direction.as_float() * m_horizontal_multiplier;
	if (m_duration.is_complete()) { collider.physics.acceleration.x = 0.f; }
}

} // namespace fornani::player
