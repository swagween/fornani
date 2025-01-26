
#include "fornani/weapon/Arsenal.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include <algorithm>

namespace arms {

Arsenal::Arsenal(automa::ServiceProvider& svc) : m_services(&svc) {}

void Arsenal::push_to_loadout(int id) {
	if (has(id)) { return; }
	loadout.push_back(std::make_unique<Weapon>(*m_services, id));
}

void Arsenal::pop_from_loadout(int id) {
	if (!has(id)) { return; }
	if (loadout.size() == 1) {
		loadout.clear();
		return;
	}
	std::erase_if(loadout, [id](auto const& g) { return g->get_id() == id; });
}

void Arsenal::reset() {
	for (auto& gun : loadout) { gun->reset(); }
}

Weapon& Arsenal::get_weapon_at(int id) { 
	auto index{0};
	auto ctr{0};
	for (auto& gun : loadout) {
		if (gun->get_id() == id) { index = ctr; }
		++ctr;
	}
	return *loadout.at(index).get();
}

bool Arsenal::has(int id) {
	for (auto& gun : loadout) {
		if (gun->get_id() == id) { return true; }
	}
	return false;
}

} // namespace arms
