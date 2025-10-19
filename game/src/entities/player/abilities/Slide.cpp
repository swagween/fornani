
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Slide.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Slide::Slide(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction)
	: Ability(svc, map, collider, direction), m_speed_multiplier{7.5f}, m_minimum_threshold{0.3f}, m_dampen{0.95f}, m_map{&map}, m_services{&svc} {
	m_type = AbilityType::slide;
	m_state = AnimState::slide;
	m_duration = util::Cooldown{512};
	m_duration.start();
	svc.soundboard.flags.player.set(audio::Player::slide);
}

void Slide::update(shape::Collider& collider, PlayerController& controller) {
	Ability::update(collider, controller);
	m_dampen = m_duration.get_cubic_normalized();
	auto prev_count = m_accumulated_speed.get_count();
	if (collider.downhill()) {
		m_duration.start();
		m_accumulated_speed.update();
	} else {
		m_accumulated_speed.update(-1);
	}
	auto super_threshold = 200;
	auto super = m_accumulated_speed.get_count() > super_threshold;
	if (m_accumulated_speed.get_count() == super_threshold && prev_count < m_accumulated_speed.get_count()) { m_services->soundboard.flags.player.set(audio::Player::super_slide); }
	collider.physics.acceleration.x = m_direction.as_float() * (m_speed_multiplier * m_dampen + static_cast<float>(m_accumulated_speed.get_count()) * 0.01f);
	if (ccm::abs(collider.physics.apparent_velocity().x) < m_minimum_threshold) {
		controller.post_slide.start();
		fail();
	}
	if (m_services->ticker.every_x_ticks(12)) {
		auto tag = super ? "super_slide" : "slide";
		m_map->active_emitters.push_back(vfx::Emitter(*m_services, collider.jumpbox.get_position(), collider.jumpbox.get_dimensions(), tag, colors::ui_white, Direction(UND::up)));
	}
}

} // namespace fornani::player
