
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Doublejump.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {

Doublejump::Doublejump(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider) : Ability(svc, map, collider), m_vertical_multiplier{-12.f} {
	m_type = AbilityType::doublejump;
	m_state = AnimState::backflip;
	svc.soundboard.flags.player.set(audio::Player::doublejump);
	auto const pos = collider.get_center() + sf::Vector2f{0.f, 12.f};
	map.effects.push_back(entity::Effect(svc, "doublejump", pos, sf::Vector2f{collider.physics.apparent_velocity().x * 0.1f, 0.f}));
	// map.spawn_emitter(svc, "doublejump", collider.get_center() + sf::Vector2f{-1.f, -1.f}, Direction{LR::left}, {2.f, 2.f});
	m_duration.start(256);
	// svc.ticker.freeze_frame(0.032f);
}

void Doublejump::update(shape::Collider& collider, PlayerController& controller) {
	if (!m_flags.test(AbilityFlags::active)) {
		controller.post_walljump.start();
		collider.physics.acceleration.y = m_vertical_multiplier;
		collider.physics.velocity.y = 0.f;
	}
	Ability::update(collider, controller);
}

} // namespace fornani::player
