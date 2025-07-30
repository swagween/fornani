
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Walljump.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Walljump::Walljump(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction) : Ability(svc, map, collider, direction), m_vertical_multiplier{-11.f}, m_horizontal_multiplier{-24.f}, m_beginning{12} {
	m_type = AbilityType::walljump;
	m_state = AnimState::backflip;
	svc.soundboard.flags.player.set(audio::Player::walljump);
	map.effects.push_back(entity::Effect(svc, "walljump", collider.get_center() + sf::Vector2f{8.f * m_direction.as_float(), 0.f}, {}));
	m_duration.start(72);
	m_beginning.start();
	m_direction.lnr = direction.left() ? LNR::right : LNR::left;
}

void Walljump::update(shape::Collider& collider, PlayerController& controller) {
	if (m_beginning.just_started()) { collider.physics.acceleration.y = m_vertical_multiplier; }
	m_beginning.update();
	if (m_beginning.is_complete()) { m_direction = controller.direction; }
	Ability::update(collider, controller);
	if (m_beginning.is_complete()) { m_flags.reset(AbilityFlags::active); }
	if (is_done()) { fail(); }
}

} // namespace fornani::player
