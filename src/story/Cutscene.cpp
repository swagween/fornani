
#include "Cutscene.hpp"
#include "../service/ServiceProvider.hpp"
#include <iostream>

namespace fornani {

Cutscene::Cutscene(automa::ServiceProvider& svc, int id, std::string_view label) : id(id) {
	auto& in_data = svc.data.cutscene[label];
	metadata.no_player = static_cast<bool>(in_data["no_player"].as_bool());
	metadata.target_state_on_end = in_data["target_state_on_end"].as<int>();
}

} // namespace fornani