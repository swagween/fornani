
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/CornerFlip.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

CornerFlip::CornerFlip(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction) : Ability(svc, map, collider, direction), m_vertical_multiplier{8.f}, m_beginning{24} {
	m_type = AbilityType::corner_flip;
	m_state = AnimState::corner_flip;
	p_force = 0.4f;
	svc.soundboard.play_sound("nani_perfect_walljump");
	collider.physics.zero();
	svc.ticker.freeze_frame(0.025f);

	m_duration.start(16);
	m_beginning.start();
}

void CornerFlip::update(shape::Collider& collider, PlayerController& controller) {
	if (m_beginning.just_started()) {
		collider.physics.velocity.y = m_vertical_multiplier;
		collider.physics.velocity.x = m_direction.as_float() * 16.f;
		controller.post_walljump.start();
	}
	m_beginning.update();
	collider.flags.movement.set(shape::Movement::walljumping);
	if (m_beginning.is_complete()) { m_direction = controller.direction; }
	Ability::update(collider, controller);
	if (m_beginning.is_complete()) { m_flags.reset(AbilityFlags::active); }
	if (is_done()) {
		fail();
		collider.flags.movement.reset(shape::Movement::walljumping);
	}
}

} // namespace fornani::player
