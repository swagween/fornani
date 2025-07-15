
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Doublejump.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Doublejump::Doublejump(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider) : Ability(svc, map, collider), m_vertical_multiplier{-12.f} {
	m_type = AbilityType::doublejump;
	m_state = AnimState::backflip;
	svc.soundboard.flags.player.set(audio::Player::jump);
	map.effects.push_back(entity::Effect(svc, "doublejump", collider.get_center() - sf::Vector2f{0.f, 8.f}, sf::Vector2f{collider.physics.apparent_velocity().x * 0.1f, 0.f}));
	m_duration.start(256);
}

void Doublejump::update(shape::Collider& collider, PlayerController& controller) {
	if (!m_flags.test(AbilityFlags::active)) {
		collider.physics.acceleration.y = m_vertical_multiplier;
		collider.physics.velocity.y = 0.f;
	}
	Ability::update(collider, controller);
}

} // namespace fornani::player
