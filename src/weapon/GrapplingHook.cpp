#include "GrapplingHook.hpp"
#include "../setup/ServiceLocator.hpp"

namespace arms {
void GrapplingHook::update() {

	if (grapple_flags.test(GrappleState::probing)) { spring.set_bob(svc::playerLocator.get().equipped_weapon().barrel_point); }
	if (grapple_flags.test(GrappleState::anchored)) {
		spring.update();
		svc::loggerLocator.get().states.set(util::State::hook_anchored);
	} else {
		svc::loggerLocator.get().states.reset(util::State::hook_anchored);
	}

	if (grapple_triggers.test(arms::GrappleTriggers::released)) {
		svc::playerLocator.get().collider.physics.acceleration = spring.variables.physics.acceleration;
		svc::playerLocator.get().collider.physics.velocity = spring.variables.physics.velocity;
		spring.variables = {};
		spring.set_bob(spring.get_anchor());
		spring.variables.physics.position = spring.get_bob();
		spring.set_anchor(svc::playerLocator.get().apparent_position);
		svc::loggerLocator.get().triggers.set(util::Trigger::hook_released);
		grapple_triggers.reset(arms::GrappleTriggers::released);
	}
	if (grapple_flags.test(arms::GrappleState::snaking)) {
		spring.set_rest_length(-80);
		spring.set_anchor(svc::playerLocator.get().apparent_position);
		spring.update();
		svc::loggerLocator.get().states.set(util::State::hook_snaking);
	} else {
		svc::loggerLocator.get().states.reset(util::State::hook_snaking);
	}

	svc::loggerLocator.get().hook_bob_position = spring.get_bob();
	svc::loggerLocator.get().hook_anchor_position = spring.get_anchor();
	svc::loggerLocator.get().hook_physics_position = spring.variables.physics.position;
}
sf::Vector2<float> GrapplingHook::probe_velocity(float speed) {
	sf::Vector2<float> ret{};
	float tweak = speed / 3;

	switch (probe_direction.inter) {
	case dir::Inter::north: ret = {0, -speed}; break;
	case dir::Inter::south: ret = {0, speed}; break;
	case dir::Inter::east: ret = {speed, -tweak}; break;
	case dir::Inter::west: ret = {-speed, -tweak}; break;
	case dir::Inter::northwest: ret = {-speed, -speed}; break;
	case dir::Inter::northeast: ret = {speed, -speed}; break;
	case dir::Inter::southwest: ret = {-speed, speed}; break;
	case dir::Inter::southeast: ret = {speed, speed}; break;
	}

	return ret;
}
} // namespace arms