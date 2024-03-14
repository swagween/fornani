
#include "Bestiary.hpp"
#include "critters/Frdog.hpp"

namespace critter {

Bestiary::Bestiary(automa::ServiceProvider& svc) : frdog(svc), hulmet(svc) {}

void Bestiary::push_critters(automa::ServiceProvider& svc, std::vector<std::pair<Critter, int>> order) {
	for (auto& crit : order) {
		for (auto i{crit.second}; i > 0; --i) {
			critter_pool.push_back(std::make_unique<Frdog>(svc));
			critter_pool.back().get()->load_data();
			critter_pool.back().get()->set_position({300, 300});
		}
	}
}

} // namespace critter
