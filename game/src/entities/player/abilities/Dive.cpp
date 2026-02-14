
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Dive.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Dive::Dive(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider) : Ability(svc, map, collider), m_request{24}, m_post_dive{8}, m_multiplier{12.f}, m_soundboard{&svc.soundboard}, m_map{&map}, m_services{&svc} {
	m_type = AbilityType::dive;
	m_state = AnimState::dive;
	m_duration.start(256);
	m_request.start();
	collider.physics.acceleration.y = m_multiplier;
	m_soundboard->flags.player.set(audio::Player::dive);
}

void Dive::update(shape::Collider& collider, PlayerController& controller) {
	m_post_dive.update();
	if (!m_flags.test(AbilityFlags::active)) {
		collider.physics.acceleration.y = m_multiplier;
		collider.physics.velocity.y = 0.f;
		m_post_dive.start();
		m_request.cancel();
	}
	Ability::update(collider, controller);
}

} // namespace fornani::player
