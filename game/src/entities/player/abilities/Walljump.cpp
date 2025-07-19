
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Walljump.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Walljump::Walljump(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction) : Ability(svc, map, collider, direction), m_vertical_multiplier{-11.f}, m_horizontal_multiplier{-24.f} {
	m_type = AbilityType::walljump;
	m_state = AnimState::backflip;
	svc.soundboard.flags.player.set(audio::Player::jump);
	map.effects.push_back(entity::Effect(svc, "jump", collider.get_center() - sf::Vector2f{0.f, 8.f}, sf::Vector2f{collider.physics.apparent_velocity().x * 0.1f, 0.f}));
	m_duration.start(12);
}

void Walljump::update(shape::Collider& collider, PlayerController& controller) {
	if (!m_flags.test(AbilityFlags::active)) {
		collider.physics.acceleration.y = m_vertical_multiplier;
		collider.physics.velocity.y = 0.f;
	}
	Ability::update(collider, controller);
	if (is_done()) { fail(); }
}

} // namespace fornani::player
