
#include "fornani/automa/StateController.hpp"
#include "fornani/graphics/Transition.hpp"

namespace fornani::automa {

void StateController::switch_rooms(int source, int destination, flfx::Transition& transition) {
	if (!transition.is_done()) { return; }
	source_id = source;
	next_state = destination;
	actions.set(Actions::trigger);
}

} // namespace fornani::automa
