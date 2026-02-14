
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Jump.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Jump::Jump(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider) : Ability(svc, map, collider), m_request{24}, m_post_jump{8}, m_multiplier{-13.76f}, m_soundboard{&svc.soundboard}, m_map{&map}, m_services{&svc} {
	m_type = AbilityType::jump;
	m_state = AnimState::rise;
	m_duration.start(256);
	m_request.start();
}

void Jump::update(shape::Collider& collider, PlayerController& controller) {
	m_post_jump.update();
	if (m_request.running()) {
		collider.flags.movement.reset(shape::Movement::jumping);
		m_request.update();
		if (controller.grounded()) {
			m_soundboard->flags.player.set(audio::Player::jump);
			m_request.cancel();
		} else {
			if (m_request.is_complete()) { m_flags.set(AbilityFlags::failed); }
			if (m_request.is_complete() && !failed()) { m_soundboard->flags.player.set(audio::Player::jump); }
			return;
		}
	}
	if (!m_flags.test(AbilityFlags::active)) {
		// this clause allows for players to jump while rolling up steep ramps
		if (collider.flags.external_state.test(shape::ExternalState::on_ramp)) {
			collider.physics.acceleration.x *= 0.5f;
			collider.physics.velocity.x *= 0.5f;
		}
		collider.physics.acceleration.y = m_multiplier;
		collider.physics.velocity.y = 0.f;
		collider.flags.movement.set(shape::Movement::jumping);
		m_map->effects.push_back(entity::Effect(*m_services, "jump", collider.get_center(), {collider.physics.apparent_velocity().x * 0.1f, 0.f}));
		m_post_jump.start();
		m_request.cancel();
	} else if (m_post_jump.is_complete()) {
		collider.flags.movement.reset(shape::Movement::jumping);
	}
	Ability::update(collider, controller);
}

} // namespace fornani::player
