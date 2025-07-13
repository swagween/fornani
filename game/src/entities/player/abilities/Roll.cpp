
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Roll.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Roll::Roll(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction) : Ability(svc, map, collider, direction), m_request{20}, m_multiplier{11.5f}, m_soundboard{&svc.soundboard} {
	m_type = AbilityType::roll;
	m_state = AnimState::roll;
	m_duration.start(64);
	m_request.start();
}

void Roll::update(shape::Collider& collider, PlayerController& controller) {
	if (m_request.running()) {
		m_request.update();
		if (m_request.is_complete() && !collider.grounded()) { m_flags.set(AbilityFlags::failed); }
		if (m_request.is_complete() && !failed()) { m_soundboard->flags.player.set(audio::Player::roll); }
		return;
	}
	Ability::update(collider, controller);
	collider.physics.acceleration.x = m_direction.as_float() * m_multiplier;
	collider.physics.velocity.x = m_direction.as_float() * m_multiplier;
}

} // namespace fornani::player
