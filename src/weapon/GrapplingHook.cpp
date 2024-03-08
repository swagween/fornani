#include "GrapplingHook.hpp"
#include "../setup/ServiceLocator.hpp"

namespace arms {
void GrapplingHook::update() {

	if (grapple_flags.test(GrappleState::anchored)) {
		spring.update();
		svc::loggerLocator.get().states.set(util::State::hook_anchored);
	} else {
		svc::loggerLocator.get().states.reset(util::State::hook_anchored);
	}
	if (grapple_triggers.test(arms::GrappleTriggers::released)) {
		//spring.reverse_anchor_and_bob();
		spring.variables = {};
		svc::loggerLocator.get().triggers.set(util::Trigger::hook_released);
	}
	if (grapple_flags.test(arms::GrappleState::snaking)) {
		spring.set_anchor(svc::playerLocator.get().apparent_position);
		spring.update();
		svc::loggerLocator.get().states.set(util::State::hook_snaking);
	} else {
		svc::loggerLocator.get().states.reset(util::State::hook_snaking);
	}

}
} // namespace arms