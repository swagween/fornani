
#include "fornani/automa/StateController.hpp"
#include "fornani/graphics/Transition.hpp"

namespace fornani::automa {

void StateController::switch_rooms(int source, int destination, graphics::Transition& transition) {
	if (!transition.is(graphics::TransitionState::black)) { return; }
	source_id = source;
	next_state = destination;
	actions.set(Actions::trigger);
}

} // namespace fornani::automa
