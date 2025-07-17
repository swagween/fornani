
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Jump.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Slide::Slide(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction) : Ability(svc, map, collider, direction), m_speed_multiplier{6.5f}, m_minimum_threshold{0.2f}, m_dampen{0.9f}, m_map{&map} {
	m_type = AbilityType::slide;
	m_state = AnimState::slide;
	m_duration.start();
	svc.soundboard.flags.player.set(audio::Player::slide);
}

void Slide::update(shape::Collider& collider, PlayerController& controller) {
	Ability::update(collider, controller);
	m_dampen = m_duration.get_cubic_normalized();
	collider.physics.acceleration.x = m_speed_multiplier * m_direction.as_float() * m_dampen;
	if (ccm::abs(collider.physics.apparent_velocity().x < m_minimum_threshold)) {
		controller.post_slide.start();
		cancel();
	}
	// m_map->effects.push_back(entity::Effect(*m_services, "slide", collider.get_center(), {collider.physics.apparent_velocity().x * 0.1f, 0.f}));
}

} // namespace fornani::player
