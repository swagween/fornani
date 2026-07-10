
#include "fornani/story/Cutscene.hpp"
#include <fornani/events/GameplayEvent.hpp>
#include "fornani/service/ServiceProvider.hpp"

namespace fornani {

Cutscene::Cutscene(automa::ServiceProvider& svc, int id, std::string_view label) : id(id) {
	auto& in_data = svc.data.cutscene[label];
	metadata.no_player = static_cast<bool>(in_data["no_player"].as_bool());
	metadata.target_state_on_end = in_data["target_state_on_end"].as<int>();
	svc.events.set_cutscene_progression_event.attach_to(p_slot, &Cutscene::set_progress, this);
}

void Cutscene::set_progress(int const to) {
	if (progress >= to) { return; }
	progress = to;
	if (debug) { NANI_LOG_DEBUG(p_logger, "set cutscene progress to {}", to); }
}

} // namespace fornani
