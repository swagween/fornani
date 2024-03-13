
#include "Bestiary.hpp"
#include "critters/Frdog.hpp"

namespace critter {

	Bestiary::Bestiary(automa::ServiceProvider& svc) : frdog(svc), hulmet(svc) { critter_pool.push_back(std::make_unique<Frdog>(svc));
	}

void Bestiary::push_critters(automa::ServiceProvider& svc, std::vector<std::pair<Critter, int>> order) {
		for (auto& crit : order) {
		for (auto i{crit.second}; i > 0; --i) { critter_pool.push_back(std::make_unique<Frdog>(svc)); }
		}
	}

} // namespace critter
