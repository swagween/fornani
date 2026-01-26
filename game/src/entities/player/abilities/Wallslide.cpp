
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Wallslide.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Wallslide::Wallslide(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction) : Ability(svc, map, collider, direction), m_speed_multiplier{14.65f}, m_map{&map}, m_services{&svc}, m_base_grav{3.5f} {
	m_type = AbilityType::wallslide;
	m_state = AnimState::wallslide;
	m_duration.start();
	svc.soundboard.repeat_sound("nani_wallslide");
}

void Wallslide::update(shape::Collider& collider, PlayerController& controller) {
	Ability::update(collider, controller);
	if (controller.wallslide_slowdown.is_complete()) {
		collider.physics.acceleration.y = m_base_grav;
		collider.physics.maximum_velocity.y = m_speed_multiplier;
	}
	auto const& left_released = m_services->controller_map.digital_action_status(config::DigitalAction::platformer_left).released;
	auto const& right_released = m_services->controller_map.digital_action_status(config::DigitalAction::platformer_right).released;
	auto const& left_pressed = m_services->controller_map.digital_action_status(config::DigitalAction::platformer_left).triggered;
	auto const& right_pressed = m_services->controller_map.digital_action_status(config::DigitalAction::platformer_right).triggered;
	if (((left_released || right_pressed) && m_direction.left()) || ((right_released || left_pressed) && m_direction.right())) { fail(); }
	if (!collider.has_left_wallslide_collision() && m_direction.left()) { fail(); }
	if (!collider.has_right_wallslide_collision() && m_direction.right()) { fail(); }
	if (collider.grounded()) { fail(); }
	if (failed()) { controller.post_wallslide.start(); }
}

} // namespace fornani::player
