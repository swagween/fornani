
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Ability.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Ability::Ability(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction) : m_direction{direction} {}

void Ability::update(shape::Collider& collider, PlayerController& controller) {
	m_duration.update();
	m_flags.set(AbilityFlags::active);
}

} // namespace fornani::player
