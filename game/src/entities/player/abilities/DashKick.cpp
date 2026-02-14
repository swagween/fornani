
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
	auto pos = direction.left() ? collider.wallslider.get_position() : collider.wallslider.get_position() + sf::Vector2f{collider.wallslider.get_dimensions().x, 0.f};
	map.spawn_effect(svc, "bright_flare", pos);
	map.spawn_emitter(svc, "dash_kick", pos, Direction{UND::up});
	map.spawn_emitter(svc, "dash_kick", pos, Direction{UND::down});
	svc.soundboard.flags.player.set(audio::Player::dash_kick);
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
