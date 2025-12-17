
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/DashKick.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

DashKick::DashKick(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction) : Ability(svc, map, collider, direction), m_horizontal_multiplier{14.f}, m_vertical_multiplier{2.f}, m_rate{2} {
	m_type = AbilityType::dash_kick;
	m_state = AnimState::dash_kick;
	m_duration.start(64);
	map.effects.push_back(entity::Effect(svc, "hit_flash", collider.get_center(), sf::Vector2f{collider.physics.apparent_velocity().x * 0.5f, 0.f}));
	svc.soundboard.flags.player.set(audio::Player::dash);
	original_gravity = collider.physics.gravity;
}

void DashKick::update(shape::Collider& collider, PlayerController& controller) {
	Ability::update(collider, controller);
	collider.physics.zero();
	collider.physics.acceleration.y = -8.f;
	collider.physics.gravity = 0.f;
	collider.flags.state.reset(shape::State::just_landed);
	if (m_duration.is_complete()) { collider.physics.gravity = original_gravity; }
}

} // namespace fornani::player
